import { initializeApp } from 'firebase/app';
import { getDatabase, ref, onValue } from 'firebase/database';

const firebaseConfig = {
  apiKey: "AIzaSyDLo9IsiIVdIMZlQqz8JEVhRrUZt5BHAQw",
  authDomain: "agrosense-e00de.firebaseapp.com",
  databaseURL: "https://agrosense-e00de-default-rtdb.firebaseio.com",
  projectId: "agrosense-e00de",
  storageBucket: "agrosense-e00de.firebasestorage.app",
  messagingSenderId: "674846785029",
  appId: "1:674846785029:web:9b6860a799cb396678a66c",
  measurementId: "G-3LQWTZK219"
};

const app = initializeApp(firebaseConfig);
export const db = getDatabase(app);

export const subscribeToPlant = (_plantId: string, callback: (data: any) => void) => {
  const currentRef = ref(db, `sensors/plantsense`);
  const historyRef = ref(db, `sensors/plantsense_history`);
  
  let currentData: any = null;
  let historyData: any = {};

  const notify = () => {
    if (currentData) {
      callback({
        current: {
          moisture: currentData.soil_moisture || 0,
          temperature: currentData.air_temp || 0,
          humidity: currentData.air_humidity || 0,
          timestamp: currentData.timestamp || Date.now(),
          ...currentData.recommendation
        },
        history: historyData
      });
    } else {
      callback(null);
    }
  };

  const unsubCurrent = onValue(currentRef, (snapshot) => {
    currentData = snapshot.val();
    notify();
  });

  const unsubHistory = onValue(historyRef, (snapshot) => {
    historyData = snapshot.val() || {};
    notify();
  });

  return () => {
    unsubCurrent();
    unsubHistory();
  };
};

