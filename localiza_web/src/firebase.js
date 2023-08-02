import { initializeApp } from "firebase/app";
import {getDatabase} from"firebase/database";
const firebaseConfig = {
  apiKey: "AIzaSyCSvjXtBu6Nx16I1qjbPVP-W4VoMkXUnjA",
  authDomain: "fb-crud-33b0f.firebaseapp.com",
  projectId: "fb-crud-33b0f",
  storageBucket: "fb-crud-33b0f.appspot.com",
  messagingSenderId: "458183089577",
  appId: "1:458183089577:web:a418aa789fed5429e8b8b8"
};

const app = initializeApp(firebaseConfig);
export const db = getDatabase(app);
