export interface SensorData {
  moisture: number;
  temperature: number;
  humidity: number;
  rawMoisture?: number;
  timestamp: number;
}

export interface RecommendationEngineState {
  healthStatus: 'Excellent' | 'Healthy' | 'Needs Attention' | 'Critical';
  recommendationText: string;
  recommendedWaterML: number;
  nextWateringEstimateHours: number;
}

export interface PlantState {
  current: SensorData & RecommendationEngineState;
  history: Record<string, SensorData>;
}

export interface PlantKnowledge {
  name: string;
  scientificName: string;
  type: string;
  difficulty: string;
  idealEnvironment: {
    temperature: { min: number; max: number; unit: string };
    humidity: { min: number; max: number; unit: string };
    soil: string;
    potDrainageRequired: boolean;
  };
  wateringRules: {
    preferredMoisture: string;
    dryingRequirement: string;
    warnings: string[];
    targetMoisturePercent: number;
    criticalMoisturePercent: number;
    overwateredMoisturePercent: number;
  };
  plantFacts: {
    growthSeason: string;
    dormancy: string;
    petSafety: string;
    fertilizerReminder: string;
    repotReminder: string;
  };
}
