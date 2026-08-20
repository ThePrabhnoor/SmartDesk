import { useState, useEffect } from 'react';
import { subscribeToPlant } from '../services/firebase';
import type { PlantState } from '../types';

export const usePlantData = (plantId: string) => {
  const [data, setData] = useState<PlantState | null>(null);
  const [loading, setLoading] = useState(true);
  const [error] = useState<string | null>(null);

  useEffect(() => {
    const unsubscribe = subscribeToPlant(plantId, (val) => {
      if (val) {
        setData(val);

      } else {
        // Handle no data
        setData(null);
      }
      setLoading(false);
    });

    return () => unsubscribe();
  }, [plantId]);

  return { data, loading, error };
};
