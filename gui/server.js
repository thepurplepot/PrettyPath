const express = require("express");
const path = require("path");
const cors = require("cors");
const fs = require("fs");
const { exec } = require("child_process");

const app = express();
const port = 3001;

app.use(cors());

app.use(express.json());
app.use(express.static(path.join(__dirname, "../data/path")));

app.post("/config", (req, res) => {
  const data = req.body;

  fs.writeFileSync(
    path.join(__dirname, "../data/path/config.json"),
    JSON.stringify(data, null, 2),
    (err) => {
      if (err) {
        console.error(err);
        res.status(500).send("Error saving config");
      } else {
        console.log("Config saved");
        res.send("Config saved");
      }
    }
  );
});

app.post("/run", (req, res) => {
  console.log("Running executable");
  exec(
    path.join(__dirname, "../Release/PrettyPath"),
    { cwd: "../" },
    (error, stdout, stderr) => {
      if (error) {
        console.error(`exec error: ${error}`);
        return res.status(500).json({ error: "Failed to run executable" });
      }
      if (stderr) {
        console.error(`stderr: ${stderr}`);
      }
      console.log(`stdout: ${stdout}`);
      res.json({ message: "Executable ran successfully", output: stdout });
    }
  );
});

app.listen(port, () => {
  console.log(`Server listening at http://localhost:${port}`);
});
