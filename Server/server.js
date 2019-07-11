const http = require("http");
const socketIO = require("socket.io");

// CONFIGURATION START

const designs = {
	"e13f9196-df52-4862-4e34-cdea5387d4b4-970e": {
		homePage: {
			log: "Home page."
		},
		pages: {
			"17c8f802-465e-4150-86e5-08ce019832e5": {
				color: [255, 0, 0],
				log: "Commande de pain."
			}
		}
	}
};

// CONFIGURATION END

const state = {
	color: [0, 0, 0]
};

const pathRegexp = /^\/embed\/([\w\-]+)\/(?:screen\/([\w\-]+)\/.*)?$/;

const server = http.createServer();
const io = socketIO(server);

io.on("connection", client => {
	console.log(
		`New connection from ${client.request.connection.remoteAddress}.`
	);

	client.emit("setColor", state.color);

	client.on("handDetected", () => {
		console.log("Hand detected.");
		client.broadcast.emit("handDetected");
	});

	client.on("handLost", () => {
		console.log("Hand lost.");
		client.broadcast.emit("handLost");
	});

	client.on("pathChanged", path => {
		console.log(`Path changed to ${path}.`);

		const match = pathRegexp.exec(path);
		if (match) {
			const design = designs[match[1]];
			if (design) {
				const pageId = typeof match[2] === "undefined" ? null : match[2];
				if (pageId !== design.currentPageId) {
					design.currentPageId = pageId;
					console.log(`Page changed to: ${pageId}.`);

					const page = pageId === null ? design.homePage : design.pages[pageId];
					if (page) {
						if (page.log) {
							console.log(page.log);
						}

						if (page.color) {
							state.color = page.color;
							io.emit("setColor", state.color);
						}

						if (page.action) {
							page.action();
						}
					}
				}
			}
		}
	});
});

const port = 3000;
server.listen(port, () => {
	console.log(`Server listening on port ${port}.`);
});
