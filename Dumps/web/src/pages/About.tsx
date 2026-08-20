import React from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '../components/ui/Card';
import { Leaf, Cpu, Database, Smartphone } from 'lucide-react';
import spiderPlantData from '../../../plants/spiderPlant.json';

const About = () => {
  return (
    <div className="space-y-6">
      <h1 className="text-3xl font-bold tracking-tight mb-6">About PlantSense AI</h1>
      
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        <div className="space-y-6">
          <Card className="glass-panel">
            <CardHeader>
              <CardTitle className="flex items-center gap-2">
                <Leaf className="text-primary" /> Plant Knowledge Base
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                <div className="p-4 bg-white/50 dark:bg-black/20 rounded-xl">
                  <h3 className="font-bold text-lg mb-1">{spiderPlantData.name} <span className="text-sm font-normal text-gray-500 italic">({spiderPlantData.scientificName})</span></h3>
                  <p className="text-sm text-gray-600 dark:text-gray-300 mb-4">{spiderPlantData.type} • {spiderPlantData.difficulty} Care</p>
                  
                  <div className="grid grid-cols-2 gap-4 text-sm">
                    <div>
                      <p className="font-semibold text-gray-500">Ideal Temp</p>
                      <p>{spiderPlantData.idealEnvironment.temperature.min}°C - {spiderPlantData.idealEnvironment.temperature.max}°C</p>
                    </div>
                    <div>
                      <p className="font-semibold text-gray-500">Ideal Humidity</p>
                      <p>{spiderPlantData.idealEnvironment.humidity.min}% - {spiderPlantData.idealEnvironment.humidity.max}%</p>
                    </div>
                    <div className="col-span-2">
                      <p className="font-semibold text-gray-500">Soil & Watering</p>
                      <p>{spiderPlantData.idealEnvironment.soil}. {spiderPlantData.wateringRules.dryingRequirement}.</p>
                    </div>
                  </div>
                </div>
              </div>
            </CardContent>
          </Card>

          <Card className="glass-panel">
            <CardHeader>
              <CardTitle>System Architecture</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                <div className="flex items-center gap-4">
                  <div className="p-3 bg-blue-500/20 text-blue-500 rounded-lg"><Cpu /></div>
                  <div>
                    <h4 className="font-semibold">ESP32 Hardware</h4>
                    <p className="text-sm text-gray-500">DHT11 & Soil Moisture Sensor measuring environmental data.</p>
                  </div>
                </div>
                <div className="flex items-center gap-4">
                  <div className="p-3 bg-orange-500/20 text-orange-500 rounded-lg"><Database /></div>
                  <div>
                    <h4 className="font-semibold">Firebase RTDB</h4>
                    <p className="text-sm text-gray-500">Realtime data syncing and historical storage.</p>
                  </div>
                </div>
                <div className="flex items-center gap-4">
                  <div className="p-3 bg-purple-500/20 text-purple-500 rounded-lg"><Smartphone /></div>
                  <div>
                    <h4 className="font-semibold">React Dashboard</h4>
                    <p className="text-sm text-gray-500">Vite, Tailwind, and Recharts powering this UI.</p>
                  </div>
                </div>
              </div>
            </CardContent>
          </Card>
        </div>
        
        <div>
           <Card className="glass-panel h-full">
            <CardHeader>
              <CardTitle>Plant Facts</CardTitle>
            </CardHeader>
            <CardContent>
              <ul className="space-y-4">
                <li className="flex flex-col p-3 bg-white/50 dark:bg-black/20 rounded-lg">
                  <span className="text-xs font-semibold uppercase tracking-wider text-gray-500">Growth Season</span>
                  <span className="font-medium">{spiderPlantData.plantFacts.growthSeason}</span>
                </li>
                <li className="flex flex-col p-3 bg-white/50 dark:bg-black/20 rounded-lg">
                  <span className="text-xs font-semibold uppercase tracking-wider text-gray-500">Dormancy</span>
                  <span className="font-medium">{spiderPlantData.plantFacts.dormancy}</span>
                </li>
                <li className="flex flex-col p-3 bg-white/50 dark:bg-black/20 rounded-lg">
                  <span className="text-xs font-semibold uppercase tracking-wider text-gray-500">Pet Safety</span>
                  <span className="font-medium text-green-600 dark:text-green-400">{spiderPlantData.plantFacts.petSafety}</span>
                </li>
                <li className="flex flex-col p-3 bg-white/50 dark:bg-black/20 rounded-lg">
                  <span className="text-xs font-semibold uppercase tracking-wider text-gray-500">Fertilizer Reminder</span>
                  <span className="font-medium">{spiderPlantData.plantFacts.fertilizerReminder}</span>
                </li>
                <li className="flex flex-col p-3 bg-white/50 dark:bg-black/20 rounded-lg">
                  <span className="text-xs font-semibold uppercase tracking-wider text-gray-500">Repotting</span>
                  <span className="font-medium">{spiderPlantData.plantFacts.repotReminder}</span>
                </li>
              </ul>
            </CardContent>
          </Card>
        </div>
      </div>
    </div>
  );
};

export default About;
