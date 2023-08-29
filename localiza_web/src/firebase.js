import { initializeApp } from "firebase/app";
import {getDatabase} from"firebase/database";
const firebaseConfig = {
  apiKey: "AIzaSyA_bU_xPHdMIRLzLrULWmv7EpkVkL9Ez2Q",
  authDomain: "projeto-localiza-388217.firebaseapp.com",
  databaseURL: "https://projeto-localiza-388217-default-rtdb.firebaseio.com",
  projectId: "projeto-localiza-388217",
  storageBucket: "projeto-localiza-388217.appspot.com",
  messagingSenderId: "434989096191",
  appId: "1:434989096191:web:9237c334d911941a7300ca"
};

const app = initializeApp(firebaseConfig);
export const db = getDatabase(app);