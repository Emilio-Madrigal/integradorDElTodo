const express = require("express");
const path = require("path");

const puerto = 3001;
const app = express();


app.use(express.json());


app.use(express.static(__dirname));

app.get("/", (req, res) => {
    res.sendFile(path.join(__dirname, "index.html"));
});


app.post("/datos", (req, res) => {
    console.log("Datos recibidos del HTML:");
    console.log(req.body);
    res.send("Datos recibidos");
});

// Iniciar servidor
app.listen(puerto, () => {
    console.log("Servidor corriendo en el puerto:", puerto);
});
