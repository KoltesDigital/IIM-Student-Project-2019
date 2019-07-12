const http = require("http");
const socketIO = require("socket.io");

// CONFIGURATION START

// Server port.
const serverPort = 3000;

const designs = {
	"839633a7-d6da-4c4f-50e3-44fafddbdfe2-f5a7": {
		homePage: {
			log: "Home page."
		},
		pages: {
			"86206a3d-f211-41f4-aad1-910ada5dad3c": {
				light: true,
				log: "Du pain a été commandé !"
			},
			"890b050a-5be1-4083-9c80-0d00354ac62a": {
				light: false,
				log: "La demande du pain a été annulé !"
			},
			"f8fa857d-46f7-4891-a101-42e1e6decfbf": {
				light: true,
				log: "De l’eau a été demandé !"
			},
			"652af495-1693-4a57-9567-84a9a44ab9bf": {
				light: false,
				log: "La demande d’eau a été annulé !"
			},
			"e504ea3d-b370-4455-bb5d-940545744da7": {
				light: true,
				log: "Le serveur a été demandé !"
			},
			"dc60e719-afec-46a6-893f-cd4a0d91166f": {
				light: false,
				log: "L’appel du serveur a été annulé !"
			},
			"b9be37b5-effb-44bb-bd65-43e3c3d109df": {
				light: true,
				log: "La table N°10 souhaite payer l’addition en espèces !"
			},
			"01d5963c-afdd-4b73-8a7d-cc62c0cffcd6": {
				light: false,
				log: "La table N°10 ne souhaite plus payer l’addition en espèces !"
			},
			"11c17e63-819b-45b5-8930-0c711b1b2260": {
				light: true,
				log: "La table N°10 souhaite payer l’addition en carte bleu !"
			},
			"70a650bb-18aa-4f9b-b754-e17c1701c7b6": {
				light: false,
				log: "La table N°10 ne souhaite plus payer l’addition en carte bleu !"
			}
		}
	}
};

// CONFIGURATION END

const state = {
	handDetected: false,
	light: false
};

const pathRegexp = /^\/embed\/([\w\-]+)\/(?:screen\/([\w\-]+)\/.*)?$/;

const server = http.createServer();
const io = socketIO(server);

io.on("connection", client => {
	console.log(
		`New connection from ${client.request.connection.remoteAddress}.`
	);

	client.emit(state.handDetected ? "handDetected" : "handLost");
	client.emit("setLight", state.light);

	client.on("handDetected", () => {
		console.log("Hand detected.");
		state.handDetected = true;
		client.broadcast.emit("handDetected");
	});

	client.on("handLost", () => {
		console.log("Hand lost.");
		state.handDetected = false;
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
						if (typeof page.log !== "undefined") {
							console.log(page.log);
						}

						if (typeof page.light !== "undefined") {
							state.light = page.light;
							console.log(`Set light: ${state.light}.`);
							io.emit("setLight", state.light);
						}
					}
				}
			}
		}
	});
});

server.listen(serverPort, () => {
	console.log(`Server listening on port ${serverPort}.`);
});
