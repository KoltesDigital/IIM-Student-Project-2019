// ==UserScript==
// @name Adobe XD hook
// @version 1
// @include https://xd.adobe.com/*
// @grant GM.xmlHttpRequest
// ==/UserScript==

setInterval(() => {
	GM.xmlHttpRequest({
		method: "POST",
		url: "http://localhost:3000/xd",
		data: location.pathname,
		headers: {
			"Content-Type": "text/plain"
		}
	});
}, 200);
