const express = require("express");
const path = require("path");
const cors = require("cors");
const fs = require("fs");
const { spawn } = require("child_process");

const app = express();
const port = 3001;
const exePath = path.join(__dirname, "../Release/PrettyPath");
const configPath = path.join(__dirname, "../config.json");
const tarnsJSONPath = path.join(__dirname, "../data/tarns.json");

app.use(cors());

app.use(express.json());
app.use(express.static(path.join(__dirname, "../data/path")));

app.post("/config", (req, res) => {
  const data = req.body;

  fs.writeFileSync(
    configPath,
    JSON.stringify(data, null, 2),
    (err) => {
      if (err) {
        console.error(err);
        res.status(500).send("Error saving config");
      }
    }
  );
  res.send("Config saved");
  console.log("Config saved");
});

app.post("/run", (_, res) => {
  console.log("Running executable");
  const child = spawn(exePath, [], {
    cwd: "../",
  });

  child.stdout.on("data", (data) => {
    console.log(`stdout: ${data}`);
    res.write(data);
  });

  child.stderr.on("data", (data) => {
    console.error(`stderr: ${data}`);
  });

  child.on("error", (error) => {
    console.error(`error: ${error}`);
    res.status(500).json({ error: "Failed to run executable" });
  });

  child.on("close", (code) => {
    console.log(`child process exited with code ${code}`);
    res.end();
  });
});

app.get("/pois", (req, res) => {
  const fileName = req.query.file;
  const filePath = path.join(__dirname, `../${fileName}`);
  if (fs.existsSync(filePath)) {
    fs.readFile(filePath, "utf8", (err, data) => {
      if (err) {
        console.error(err);
        res.status(500).send("Error reading POIs");
      } else {
        console.log("POI file read");
        res.send(data);
      }
    });
  } else {
    res.status(404).send("POI file not found");
  }
});

app.post("/tarns", (req, res) => {
  const tarns = req.body;

  fs.writeFileSync(tarnsJSONPath, JSON.stringify(tarns, null, 2), (err) => {
    if (err) {
      console.error(err);
      res.status(500).send("Error saving config");
    }
  });
  res.send("Tarns saved");
  console.log("Tarns saved");
});

app.listen(port, () => {
  console.log(`Server listening at http://localhost:${port}`);
});
