import '../App.css';
import { db } from "../firebase";
import { set, ref, onValue, remove, update } from "firebase/database";
import { useState, useEffect } from "react";

function Beacon() {
  const [todo, setTodo] = useState("");
  const [address, setAddress] = useState("");
  const [todos, setTodos] = useState([]);
  const [isEdit, setIsEdit] = useState(false);

  useEffect(() => {
    const beaconsRef = ref(db, '/localiza/dictionary/beacons');

    onValue(beaconsRef, (snapshot) => {
      const data = snapshot.val();
      if (data !== null) {
        const beaconsArray = Object.entries(data).map(([mac, nome]) => ({
          mac,
          nome
        }));
        setTodos(beaconsArray);
      }
    });
  }, []);

  const handleTodoChange = (e) => {
    setTodo(e.target.value);
  };

  const handleAddressChange = (e) => {
    setAddress(e.target.value);
  };

  const writeToDatabase = () => {
    update(ref(db, `/localiza/dictionary/beacons`), {
      [address]: todo,
    });
    setTodo("");
  };

  const handleUpdate = (todo) => {
    setIsEdit(true);
    setAddress(todo.mac);
    setTodo(todo.nome);
  };

  const handleSubmitChange = () => {
    remove(ref(db, `/localiza/dictionary/beacons/${address}`))
      .then(() => {
        const updateData = {
          [address]: todo,
        };
        return update(ref(db, `/localiza/dictionary/beacons`), updateData);
      })
      .then(() => {
        setAddress("");
        setTodo("");
        setIsEdit(false);
      })
      .catch((error) => {
        console.error("Erro ao atualizar os dados:", error);
      });
  };
  
  const handleDelete = (todo) => {
    remove(ref(db, `/localiza/dictionary/beacons/${todo.mac}`));
  };

  return (
    <div className="App">
      <div className="input-container">
        <input
          type="text"
          value={address}
          onChange={handleAddressChange}
          placeholder="Endereço MAC"
        />
        <input
          type="text"
          value={todo}
          onChange={handleTodoChange}
          placeholder="Nome do Beacon"
        />
        {isEdit ? (
          <div className="button-group">
            <button onClick={handleSubmitChange}>Atualizar</button>
            <button
              onClick={() => {
                setIsEdit(false);
                setTodo("");
                setAddress("");
              }}
            >
              Cancelar
            </button>
          </div>
        ) : (
          <button className="submit" onClick={writeToDatabase}>Adicionar Beacon</button>
        )}
      </div>

      <div className="beacon-list">
        {todos.map((beacon) => (
          <div className="beacon-item" key={beacon.mac}>
            <h2>{beacon.mac + " -> " + beacon.nome}</h2>
            <div className="button-group">
              <button onClick={() => handleUpdate(beacon)}>Editar</button>
              <button onClick={() => handleDelete(beacon)}>Excluir</button>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}

export default Beacon;