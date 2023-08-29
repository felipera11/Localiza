import React from "react";
import { Link } from "react-router-dom";

function Home() {
  return (
    <div className="home-container">
      <div className="button-container">
        <h1>Beacon</h1>
        <Link to="/beacon">
          <button className="submit">Ir para Beacon management</button>
        </Link>
      </div>
      <div className="button-container">
        <h1>Scanner</h1>
        <Link to="/scanner">
          <button className="submit">Ir para Scanner management</button>
        </Link>
      </div>
    </div>
  );
}

export default Home;