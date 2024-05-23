import React from "react";

function RunButton() {
  const handleClick = async () => {
    try {
      const response = await fetch("http://localhost:3001/run", { method: "POST" });
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      const data = await response.json();
      console.log(data);
    } catch (error) {
      console.error("There was a problem with the fetch operation:", error);
    }
  };

  return <button onClick={handleClick}>Run</button>;
}

export default RunButton;
