# Vendored library archives

Each third party library OpenVSP builds from source is kept here as an archive,
referenced by its file name from the matching `Libraries/cmake/External_<Name>.cmake`.
The name identifies the exact source either way: it is either a zip named
`<name>-<first 12 characters of the commit sha>.zip`, made from a git commit as
described below, or a library's official release archive with the version number
in its name, such as `eigen-5.0.1.zip`.

## Making one from a commit

The archives are `git archive` output, so the tree inside is prefixed with the
**full** sha while the file name carries the first twelve.  From a clone of the
library, at the commit you want:

    SHA=$( git rev-parse <commit> )
    git archive --format=zip --prefix=<Name>-$SHA/ -o <openvsp>/Libraries/<Name>-${SHA:0:12}.zip $SHA

Then point the cmake at it and delete the archive it replaces:

    Libraries/cmake/External_<Name>.cmake:   URL ${CMAKE_SOURCE_DIR}/<Name>-<first 12>.zip

Two details that are easy to get wrong and that nothing checks:

- the prefix inside the archive is the **full** sha, not the twelve of the file
  name.  Nothing in the build verifies it, and a mismatch shows up only as a
  confusing path.
- `git archive` writes the commit sha as the zip's archive comment, so
  `unzip -z <file>` tells you which commit an archive really came from.  Use it
  to check an archive rather than trusting its name.

## Where the bump belongs in a branch

Put the commit that swaps the archive **before** the first commit that needs
anything new in it -- by new API used, or by a behaviour change it relies on.
Otherwise every commit between the two fails to configure from a clean checkout,
which nobody notices while building against a working tree of the library.
