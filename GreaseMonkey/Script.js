// ==UserScript==
// @name Adobe XD hook
// @version 1
// @include https://xd.adobe.com/*
// @require https://cdnjs.cloudflare.com/ajax/libs/socket.io/2.2.0/socket.io.slim.js
// ==/UserScript==

// CONFIGURATION START

// How long it takes to show or hide the webpage.
const fadeDuration = "300ms";

// Server hostname or IP address.
const serverHost = "localhost";

// Server port.
const serverPort = 3000;

// CONFIGURATION END

const maskElement = document.createElement("div");
document.body.appendChild(maskElement);
Object.assign(maskElement.style, {
	background: "black",
	height: "100%",
	left: 0,
	"pointer-events": "none",
	position: "fixed",
	opacity: 1,
	top: 0,
	transition: "opacity " + fadeDuration,
	width: "100%"
});

const socket = io("http://" + serverHost + ":" + serverPort);

let path = null;

socket.on("connect", () => {
	console.log("Connected.");
	path = null;
});

socket.on("disconnect", () => {
	console.log("Disconnected.");
});

socket.on("handDetected", () => {
	console.log("Hand detected.");
	maskElement.style.opacity = 0;
});

socket.on("handLost", () => {
	console.log("Hand lost.");
	maskElement.style.opacity = 1;
});

setInterval(() => {
	let newPath = location.pathname;

	if (newPath !== path) {
		socket.emit("pathChanged", newPath);
	}

	path = newPath;
}, 100);
