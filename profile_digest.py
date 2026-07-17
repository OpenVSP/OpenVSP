#!/usr/bin/env python3
"""Digest macOS /usr/bin/sample call-tree output into a compact report.

Usage:
  profile_digest.py capture.txt [extra_watch_regex ...]
  profile_digest.py old.txt new.txt --diff [extra_watch_regex ...]

Designed so a profile can be analyzed from its digest alone:
- main-thread scoped; idle measured at blocking LEAVES (on macOS, FLTK dispatches
  events inside Fl_Darwin_System_Driver::wait, so "total minus wait" is wrong)
- exclusive (self) hotspots aggregated by cleaned symbol
- inclusive subtree totals for a watchlist, counting top-most occurrences only
- heaviest compressed call paths
"""

import re
import sys

IDLE_LEAVES = (
    "mach_msg2_trap", "__select", "kevent", "__workq_kernreturn",
    "__psynch_cvwait", "__semwait_signal", "__ulock_wait", "semaphore_wait_trap",
    "mach_msg_trap", "__sigsuspend",
)

DEFAULT_WATCH = [
    r"Vehicle::Update\b", r"Vehicle::UpdateBBox", r"Geom::Update\b",
    r"UpdateSurf", r"UpdateMainTessVec", r"UpdateTessVec", r"UpdateSplitTesselate",
    r"UpdateDrawObj", r"UpdateDegenDrawObj", r"ApplySymm", r"TireToBogie",
    r"BuildFeatureLines", r"BuildLCurve", r"CalcTexCoords",
    r"SkinRibs", r"SkinCubicSpline", r"skinning_surface_creator",
    r"piecewise_general_creator", r"SparseLU", r"de_casteljau",
    r"ParmChanged", r"ExecuteScript", r"ClearXSecSurfs",
    r"Fl::flush", r"UpdateAllScreens", r"_loadXSecData", r"CompGeom",
    r"Intersect", r"Tessellate",
    # CFD / FEA mesh worker paths
    r"cfdmesh_thread_fun", r"GenerateMesh", r"InitMesh", r"RemoveInteriorTris",
    r"BuildTargetMap", r"Remesh", r"BuildMesh", r"WriteMesh", r"MatchWakes",
    r"Intersect\(", r"Triangulate", r"BuildGrid", r"BinaryAdapt", r"CalcMatchNodes",
    r"SubTag", r"BuildEdges", r"BuildNodes", r"ConnectBorder",
]

LINE_RE = re.compile(r"^(\s+[+!:| ]*)(\d+) (.*)$")
THREAD_RE = re.compile(r"^\s+(\d+) Thread_\d+")


def clean(name):
    """Strip (in lib), template args, arg lists, offsets from a symbol."""
    name = re.sub(r"\s+\(in [^)]*\).*$", "", name)
    name = re.sub(r"\s+\+\s+\d.*$", "", name)
    # strip balanced template args
    out = []
    depth = 0
    for ch in name:
        if ch == "<":
            depth += 1
        elif ch == ">":
            depth = max(0, depth - 1)
        elif depth == 0:
            out.append(ch)
    name = "".join(out)
    # strip trailing argument list
    i = name.rfind("(")
    if i > 0:
        name = name[:i]
    return name.strip()


class Node:
    __slots__ = ("count", "name", "children", "busy")

    def __init__(self, count, name):
        self.count = count
        self.name = name
        self.children = []
        self.busy = 0  # subtree count minus idle blocking-leaf samples


def _iter_thread_blocks(path):
    """Yield (header_line, sample_count, body_lines) for each thread in the report."""
    header = None
    count = 0
    body = []
    for ln in open(path, errors="replace"):
        m = THREAD_RE.match(ln)
        if m and "Thread_" in ln:
            if header is not None:
                yield header, count, body
            header = ln.rstrip("\n")
            count = int(m.group(1))
            body = []
        elif header is not None:
            body.append(ln)
    if header is not None:
        yield header, count, body


def _build_tree(body):
    roots = []
    stack = []  # (depth, node)
    for ln in body:
        m = LINE_RE.match(ln)
        if not m:
            continue
        depth = len(m.group(1))
        node = Node(int(m.group(2)), clean(m.group(3)))
        while stack and stack[-1][0] >= depth:
            stack.pop()
        if stack:
            stack[-1][1].children.append(node)
        else:
            roots.append(node)
        stack.append((depth, node))
    return roots


GENERIC_ENTRY = ("start", "thread_start", "start_wqthread", "_pthread_start",
                 "_dispatch_worker_thread", "__workq", "_pthread_wqthread")


def _thread_ident(body, nscan=25):
    """Return (label, searchtext) identifying a thread from its early frames.
    label is the first application (in vsp) frame past the generic OS entry points;
    searchtext concatenates the first nscan frames so a worker can be matched by a
    function deep under thread_start (e.g. cfdmesh_thread_fun)."""
    label = ""
    parts = []
    seen = 0
    for ln in body:
        m = LINE_RE.match(ln)
        if not m:
            continue
        raw = m.group(3)
        parts.append(raw)
        name = clean(raw)
        if not label and "(in vsp)" in raw and name not in GENERIC_ENTRY:
            label = name
        seen += 1
        if seen >= nscan:
            break
    if not label:
        for raw in parts:
            n = clean(raw)
            if n not in GENERIC_ENTRY:
                label = n
                break
    return label, " ".join(parts)


def parse_thread(path, want="Main Thread"):
    """Return (roots, total, label) for the first thread whose header, or any of its
    first frames, contains `want` (case-insensitive).  Worker threads (e.g.
    cfdmesh_thread_fun) match on a frame a few levels under thread_start."""
    wl = want.lower()
    for header, count, body in _iter_thread_blocks(path):
        label, searchtext = _thread_ident(body)
        if wl in header.lower() or wl in searchtext.lower():
            return _build_tree(body), count, (label or header.strip())
    return [], 0, ""


def list_threads(path):
    print("== threads (sample count : identifying frame)")
    rows = []
    for header, count, body in _iter_thread_blocks(path):
        label, _ = _thread_ident(body)
        rows.append((count, label or header.split(":", 1)[-1].strip()))
    for count, label in sorted(rows, key=lambda r: -r[0])[:30]:
        print("  %9d  %s" % (count, label[:88]))


def walk_exclusive(node, excl, idle_box):
    child_sum = sum(c.count for c in node.children)
    self_cnt = node.count - child_sum
    self_idle = 0
    if self_cnt > 0:
        excl[node.name] = excl.get(node.name, 0) + self_cnt
        if any(k in node.name for k in IDLE_LEAVES):
            idle_box[0] += self_cnt
            self_idle = self_cnt
    child_busy = 0
    for c in node.children:
        child_busy += walk_exclusive(c, excl, idle_box)
    node.busy = node.count - self_idle - (child_sum - child_busy)
    return node.busy


def walk_watch(node, regexes, inherited, totals):
    matched = set(i for i, r in enumerate(regexes) if r.search(node.name))
    new = matched - inherited
    for i in new:
        totals[i] += node.count
    inh = inherited | matched
    for c in node.children:
        walk_watch(c, regexes, inh, totals)


def hot_paths(roots, busy, nmax=8, min_frac=0.02):
    """Heaviest root-to-significant-node chains, compressed."""
    paths = []

    def descend(node, trail):
        trail = trail + [node]
        # significant children of this node, by busy (idle-subtracted) count
        sig = [c for c in node.children if c.busy >= max(1, min_frac * busy)]
        if len(sig) == 1 and sig[0].busy >= 0.7 * node.busy:
            descend(sig[0], trail)
            return
        # emit this node as a path terminus, then recurse into significant children
        interesting = [n for n in trail if not n.name.startswith(("start", "main", "Fl::run",
                        "-[", "__", "_dispatch", "CFRunLoop", "RunCurrent", "ReceiveNext",
                        "_DPS", "NSApp", "mach_msg", "Fl_Darwin_System_Driver::wait"))]
        if interesting:
            label = " > ".join(n.name for n in interesting[-4:])
            paths.append((node.busy, label))
        for c in sig:
            descend(c, trail)

    for r in roots:
        descend(r, [])
    # dedupe by label keeping max
    best = {}
    for cnt, label in paths:
        if label not in best or best[label] < cnt:
            best[label] = cnt
    ranked = sorted(best.items(), key=lambda kv: -kv[1])
    # drop paths that are prefixes of a heavier deeper path with similar count
    out = []
    for label, cnt in ranked:
        if any(label != l2 and label in l2 and cnt <= c2 * 1.1 for l2, c2 in ranked[:20]):
            continue
        out.append((cnt, label))
        if len(out) >= nmax:
            break
    return out


def analyze(path, want="Main Thread"):
    roots, total, label = parse_thread(path, want)
    excl = {}
    idle_box = [0]
    for r in roots:
        walk_exclusive(r, excl, idle_box)
    idle = idle_box[0]
    busy = max(1, total - idle)
    return roots, total, idle, busy, excl, label


def report(path, extra_watch, want="Main Thread"):
    roots, total, idle, busy, excl, label = analyze(path, want)
    print("== %s" % path)
    if not roots:
        print("no thread matched '%s' -- try --list-threads" % want)
        return
    print("thread [%s]  samples %d   idle (blocking leaves) %d   BUSY %d (%.0f%%)"
          % (label, total, idle, busy, 100.0 * busy / max(1, total)))

    print("\n-- exclusive hotspots (% of busy)")
    shown = 0
    for name, cnt in sorted(excl.items(), key=lambda kv: -kv[1]):
        if any(k in name for k in IDLE_LEAVES):
            continue
        if cnt < 0.005 * busy or shown >= 15:
            break
        print("  %6.1f%%  %6d  %s" % (100.0 * cnt / busy, cnt, name[:90]))
        shown += 1

    regexes = [re.compile(p) for p in DEFAULT_WATCH + list(extra_watch)]
    totals = [0] * len(regexes)
    for r in roots:
        walk_watch(r, regexes, set(), totals)
    print("\n-- inclusive watchlist (top-most subtree totals, % of busy)")
    order = sorted(range(len(regexes)), key=lambda i: -totals[i])
    for i in order:
        if totals[i] <= 0:
            continue
        print("  %6.1f%%  %6d  %s" % (100.0 * totals[i] / busy, totals[i],
                                      (DEFAULT_WATCH + list(extra_watch))[i]))

    print("\n-- heaviest call paths")
    for cnt, label in hot_paths(roots, busy):
        print("  %6.1f%%  %6d  %s" % (100.0 * cnt / busy, cnt, label[:150]))


def diff(path_a, path_b, extra_watch, want="Main Thread"):
    _, tot_a, idle_a, busy_a, excl_a, _ = analyze(path_a, want)
    _, tot_b, idle_b, busy_b, excl_b, _ = analyze(path_b, want)
    print("== DIFF [%s]  A=%s (busy %d)   B=%s (busy %d)" % (want, path_a, busy_a, path_b, busy_b))
    print("   values are % of busy in each capture; delta = B - A")
    names = set(excl_a) | set(excl_b)
    rows = []
    for n in names:
        if any(k in n for k in IDLE_LEAVES):
            continue
        pa = 100.0 * excl_a.get(n, 0) / busy_a
        pb = 100.0 * excl_b.get(n, 0) / busy_b
        if max(pa, pb) < 0.5:
            continue
        rows.append((pb - pa, pa, pb, n))
    rows.sort(key=lambda r: -abs(r[0]))
    print("\n-- largest exclusive share changes")
    for d, pa, pb, n in rows[:20]:
        print("  %+6.1f%%  A %5.1f%% -> B %5.1f%%  %s" % (d, pa, pb, n[:80]))


def main():
    args = [a for a in sys.argv[1:]]
    do_diff = "--diff" in args
    if do_diff:
        args.remove("--diff")
    do_list = "--list-threads" in args
    if do_list:
        args.remove("--list-threads")
    # --thread NAME  (default Main Thread); NAME may be a substring of the thread's
    # header or its root frame (e.g. "cfdmesh" for the cfdmesh_thread_fun worker).
    want = "Main Thread"
    if "--thread" in args:
        i = args.index("--thread")
        want = args[i + 1]
        del args[i:i + 2]
    files = [a for a in args if a.endswith(".txt")]
    extra = [a for a in args if not a.endswith(".txt")]
    if do_list:
        for f in files:
            list_threads(f)
    elif do_diff and len(files) == 2:
        diff(files[0], files[1], extra, want)
    elif len(files) >= 1:
        for f in files:
            report(f, extra, want)
    else:
        print(__doc__)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
