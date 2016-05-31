
////////////////////Please leave this notice////////////////////
//
//	DropDown Menu 1.0
//	By Evgeny Novikov (java@aladin.ru)
//	http://java.skyteam.ru
//	It works only with IE5.0(++) and Netscape6.0(++)
//	Free to use!
//
////////////////////Last modified 2002-03-05////////////////////

//	Modify following four lines to customize your menu
var tdColor = "#FFFFFF";		// menu item text color
var tdBgColor = "#6060A0";		// menu item background color
var hlColor = "#000000";		// highlight text color
var hlBgColor = "#9595BD";		// highlight background color
//	After change, modify same values in your *.css file

var md = 250;
var ti = -1;
var oTd = new Object;
oTd = null;

function doMenu(td) {
	clearTimeout(ti);
	td.style.backgroundColor = hlBgColor;
	td.style.color = hlColor;
	var i;
	var sT = "";
	var tda = new Array();
	tda = td.id.split("_");
	if (oTd != null) {
		var tdo = new Array();
		tdo = oTd.id.split("_");
		for (i = 1; i < tdo.length; i++) {
			sT += "_" + tdo[i];
			if (tdo[i] != tda[i]) {
				document.getElementById("td" + sT).style.backgroundColor = tdBgColor;
				document.getElementById("td" + sT).style.color = tdColor;
				if (document.getElementById("tbl" + sT) != null)
					document.getElementById("tbl" + sT).style.visibility = "hidden";
			}
		}			
	}
	oTd = td;
	sT = "tbl";
	for (i = 1; i < tda.length; i++)
		sT += "_" + tda[i];
	if (document.getElementById(sT) != null)
		document.getElementById(sT).style.visibility = "visible";

}

function clearMenu() {
	if (oTd != null) {
		var tdo = new Array();
		tdo = oTd.id.split("_");
		var sT = "";
		for (var i = 1; i < tdo.length; i++) {
			sT += "_" + tdo[i];
			document.getElementById("td" + sT).style.backgroundColor = tdBgColor;
			document.getElementById("td" + sT).style.color = tdColor;
			if (document.getElementById("tbl" + sT) != null)
				document.getElementById("tbl" + sT).style.visibility = "hidden";
		}
		oTd = null;
	}
}

function runMenu(strURL) {
	if (strURL.charAt(0) == '%') 
		window.open(strURL.substr(1));
	else
		location.href = strURL;
}

var tt = "";
var sT = "";
var pT = new Array();
var tA = new Array();

function getCoord(st) {
	tA = st.split("_");
	if (tA.length > 2) {
		tA = tA.slice(0,-1);
		tt = tA.join("_");
		return (document.getElementById("tbl" + tt).offsetTop + document.getElementById("td" + st).offsetTop - 1) + "px;left:" +
			(document.getElementById("tbl" + tt).offsetLeft + document.getElementById("td" + st).offsetWidth - 2) + "px\">";
	}
	return (document.getElementById("mainmenu").offsetTop + document.getElementById("td" + st).offsetHeight - 10) + "px;left:" +
		(document.getElementById("mainmenu").offsetLeft + document.getElementById("td" + st).offsetLeft + 5) + "px\">";
}

function isDefined(varname) {
	return eval("typeof(" + varname + ") != \"undefined\"");
}

var sH = "<table class=\"menu\" id=\"mainmenu\" cellspacing=\"0\"  width=\"100%\"><tr>";
var p = 0;
var j = 0;

while (isDefined("td_" + ++j)) {
	sH += "<td id=\"td_" + j + "\" onmouseover=\"doMenu(this)\" onmouseout=\"ti=setTimeout('clearMenu()',md)\"";
	sH += (isDefined("url_" + j)) ? " onclick=\"runMenu('" + eval("url_" + j) + "')\">" : ">";
	sH += eval("td_" + j) + "</td>";
	if (isDefined("td_" + j + "_1"))
		pT[p++] = "_" + j;
}

sH += "<td class=\"Banner\" align=\"right\" width=\"100%\"><img src=\"../Images/_Project_Logo.gif\" align=\"absmiddle\"></td>";
sH += "</tr></table>";
document.write(sH);

for (var q = 0; typeof(pT[q]) != "undefined"; q++) {
	sT = pT[q];
	sH = "";
	j = 0;
	sH += "<table class=\"menudrop\" id=\"tbl" + sT + "\" cellspacing=\"0\" style=\"top:" + getCoord(sT);
	while (isDefined("td" + sT + "_" + ++j)) {
		sH += "<tr><td id=\"td" + sT + "_" + j + "\" onmouseover=\"doMenu(this)\" onmouseout=\"ti=setTimeout('clearMenu()',md)\"";
		sH += (isDefined("url" + sT + "_" + j)) ? " onclick=\"runMenu('" + eval("url" + sT + "_" + j) + "')\">" : ">";
		sH += eval("td" + sT + "_" + j) + "</td></tr>";
		if (isDefined("td" + sT + "_" + j + "_1"))
			pT[p++] = sT + "_" + j;
	}
	sH += "</table>";
	document.write(sH);
}
document.getElementById("mainmenu").style.visibility = "visible";

