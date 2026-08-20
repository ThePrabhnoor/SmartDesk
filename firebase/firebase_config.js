// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";
import { getAnalytics } from "firebase/analytics";
// TODO: Add SDKs for Firebase products that you want to use
// https://firebase.google.com/docs/web/setup#available-libraries

// Your web app's Firebase configuration
// For Firebase JS SDK v7.20.0 and later, measurementId is optional
const firebaseConfig = {
  apiKey: "AIzaSyA5JsnN7mjxuzSRJbzUpJbGhJ2uFHSeaJQ",
  authDomain: "smart-desk1.firebaseapp.com",
  projectId: "smart-desk1",
  storageBucket: "smart-desk1.firebasestorage.app",
  messagingSenderId: "1030498920860",
  appId: "1:1030498920860:web:0523134d4cfcd859f10059",
  measurementId: "G-B906KS9FPD"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const analytics = getAnalytics(app);
