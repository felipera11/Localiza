import "./App.css";
import { BrowserRouter as Router, Route, Routes } from "react-router-dom";
import Home from "./pages/Home";
import Beacon from "./pages/Beacon";
import Scanner from "./pages/Scanner";

function App() {
  return (
    <div className="App">
      <Router>
        <Routes>
          <Route path="/" element={<Home />} />
          <Route path="/beacon" element={<Beacon />} />
          <Route path="/scanner" element={<Scanner />} />
        </Routes>
      </Router>
    </div>
  );
}

export default App;