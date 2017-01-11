/*
 * Expandable list implementation.
 * by David Lindquist <first name><at><last name><dot><net>
 * See:
 * http://www.gazingus.org/html/DOM-Scripted_Lists_Revisited.html
 * Modifies lists so that sublists can be hidden and shown by means of
 * a switch. The switch is a node inserted into the DOM tree as the
 * first child of the list item containing the sublist.
 */

// The script will only be applied to lists containing this class name,
// e.g.: <ul class="foo expandable">...</ul>.
var CLASS_NAME = "expandable";

// This value is the assumed initial display style for a sublist when it cannot
// be determined by other means. See below.
var DEFAULT_DISPLAY = "none";

// The namespace to use when using this script in an XML context.
var XMLNS = "http://www.w3.org/1999/xhtml";

// The beginning of the title text for the switch when the sublist is collapsed.
var CLOSED_PREFIX = "Expand list: ";

// The beginning of the title text for the switch when the sublist is expanded.
var OPENED_PREFIX = "Collapse list: ";

/******************************************************************************/

// Returns a copy of a string with leading and trailing whitespace removed.
String.prototype.trim = function() {
    return this.replace(/^\s+/, "").replace(/\s+$/, "");
}

// Walks the DOM tree starting at a given root element. Returns an
// array of nodes of the specified type and conforming to the criteria
// of the given filter function. The filter should return a boolean.
function getNodesByType(root, type, filter) {
    var node = root;
    var nodes = [];
    var next;

    while (node != null) {
        if (node.hasChildNodes())
            node = node.firstChild;
        else if (node != root && null != (next = node.nextSibling))
            node = next;
        else {
            next = null;
            for ( ; node != root; node = node.parentNode) {
                next = node.nextSibling;
                if (next != null) break;
            }
            node = next;
        }
        if (node != null && node.nodeType == type && filter(node))
            nodes.push(node);
    }
    return nodes;
}

// Simulates the innerText property of IE and other browsers.
// Mozilla/Firefox need this.
function getInnerText(node) {
    if (node == null || node.nodeType != 1)
        return;
    var text = "";
    var textnodes = getNodesByType(node, 3, function() { return true; });
    for (var i = 0; i < textnodes.length; i++)
        text += textnodes[i].data;
    return text;
}

function initExpandableLists() {
    if (!document.getElementsByTagName) return;

    // Top-level function to accommodate browsers that do not register
    // a click event when a link is activated by the keyboard.
    switchNode = function(id) {
        var node = document.getElementById(id);
        if (node && /^switch /.test(node.className)) node.onclick();
    }

    // Top-level function to be assigned as the event handler for the
    // switch. This could have been bound to the handler as a closure,
    // but closures are associated with memory leak problems in IE.
    actuate = function() {
        var sublist = this.parentNode.getElementsByTagName("ul")[0] ||
                      this.parentNode.getElementsByTagName("ol")[0];
        if (sublist.style.display == "block") {
            sublist.style.display = "none";
            this.firstChild.data = "+";
            this.className = "switch off";
            this.title = this.title.replace(OPENED_PREFIX, CLOSED_PREFIX);
        } else {
            sublist.style.display = "block";
            this.firstChild.data = "-";
            this.className = "switch on";
            this.title = this.title.replace(CLOSED_PREFIX, OPENED_PREFIX);
        }
        return false;
    }

    // Create switch node from which the others will be cloned.
    if (typeof document.createElementNS == "function")
        var template = document.createElementNS(XMLNS, "a");
    else
        var template = document.createElement("a");
    template.appendChild(document.createTextNode(" "));

    var list, i = 0, j = 0;
    var pattern = new RegExp("(^| )" + CLASS_NAME + "( |$)");

    while ((list = document.getElementsByTagName("ul")[i++]) ||
           (list = document.getElementsByTagName("ol")[j++]))
    {
        // Only lists with the given class name are processed.
        if (pattern.test(list.className) == false) continue;

        var item, k = 0;
        while ((item = list.getElementsByTagName("li")[k++])) {
            var sublist = item.getElementsByTagName("ul")[0] ||
                          item.getElementsByTagName("ol")[0];
            // No sublist under this list item. Skip it.
            if (sublist == null) continue;

            // Attempt to determine initial display style of the
            // sublist so the proper symbol is used.
            var symbol;
            switch (sublist.style.display) {
            case "none" : symbol = "+"; break;
            case "block": symbol = "-"; break;
            default:
                var display = DEFAULT_DISPLAY;
                if (sublist.currentStyle) {
                    display = sublist.currentStyle.display;
                } else if (document.defaultView &&
                           document.defaultView.getComputedStyle &&
                           document.defaultView.getComputedStyle(sublist, ""))
                {
                    var view = document.defaultView;
                    var computed = view.getComputedStyle(sublist, "");
                    display = computed.getPropertyValue("display");
                }
                symbol = (display == "none") ? "+" : "-";
                // Explicitly set the display style to make sure it is
                // set for the next read. If it is somehow the empty
                // string, use the default value from the (X)HTML DTD.
                sublist.style.display = display || "block";
                break;
            }

            // This bit attempts to extract some text from the first
            // child node of the list item to append to the title
            // attribute of the switch.
            var child = item.firstChild;
            var text = "";
            while (child) {
                if (child.nodeType == 3 && "" != child.data.trim()) {
                    text = child.data;
                    break;
                } else if (child.nodeType == 1 &&
                           !/^[ou]l$/i.test(child.tagName))
                {
                    text = child.innerText || getInnerText(child);
                    break;
                }
                child = child.nextSibling;
            }

            var actuator = template.cloneNode(true);
            // a reasonably unique ID
            var uid = "switch" + i + "-" + j + "-" + k;
            actuator.id = uid;
            actuator.href = "javascript:switchNode('" + uid + "')";
            actuator.className = "switch " + ((symbol == "+") ? "off" : "on");
            actuator.title = ((symbol == "+")
                              ? CLOSED_PREFIX : OPENED_PREFIX) + text.trim();
            actuator.firstChild.data = symbol;
            actuator.onclick = actuate;
            item.insertBefore(actuator, item.firstChild);
        }
    }
}

// Props to Simon Willison:
// http://simon.incutio.com/archive/2004/05/26/addLoadEvent
var oldhandler = window.onload;
window.onload = (typeof oldhandler == "function")
    ? function() { oldhandler(); initExpandableLists(); } : initExpandableLists;
