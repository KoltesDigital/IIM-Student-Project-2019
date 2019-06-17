const bodyParser = require("body-parser");
const express = require("express");
const http = require("http");
const socketIO = require("socket.io");

const state = {
	color: [0, 0, 0]
};

const designs = {
	"e13f9196-df52-4862-4e34-cdea5387d4b4-970e": {
		pages: {
			// Commande de pain
			"17c8f802-465e-4150-86e5-08ce019832e5": {
				action: () => {
					// Allumer les leds en rouge.
					state.color = [255, 0, 0];
					io.emit("setColor", state.color);
				}
			}
		}
	}
};

const app = express();

app.use(bodyParser.text());

app.use("/xd", (req, res, next) => {
	const regexp = /^\/embed\/([\w\-]+)\/(?:screen\/([\w\-]+)\/.*)?$/;
	const match = regexp.exec(req.body);
	if (match) {
		const design = designs[match[1]];
		if (design) {
			const pageId = typeof match[2] === "undefined" ? null : match[2];
			if (pageId !== design.currentPageId) {
				design.currentPageId = pageId;
				console.log(`New page id: ${pageId}.`);

				const page = pageId === null ? design.homePage : design.pages[pageId];
				if (page) {
					if (page.action) {
						page.action();
					}
				}
			}
		}
	}
	res.sendStatus(204);
});

const server = http.createServer(app);
const io = socketIO(server);

io.on("connection", client => {
	// Envoi de l'état de l'application à la carte électronique.
	client.emit("setColor", state.color);
});

const port = 3000;
server.listen(port, () => {
	console.log(`Server listening on port ${port}.`);
});
