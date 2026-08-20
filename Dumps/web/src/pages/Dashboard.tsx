import React, { useEffect, useRef, useState } from 'react';
import { usePlantData } from '../hooks/usePlantData';
import { Card, CardContent, CardHeader, CardTitle } from '../components/ui/Card';
import { Droplet, Thermometer, Wind, AlertCircle, Clock, Activity } from 'lucide-react';
import { format, formatDistanceToNow } from 'date-fns';

const Dashboard = () => {
  const { data, loading, error } = usePlantData('spiderPlant');
  const [notificationPermission, setNotificationPermission] = useState(Notification.permission);
  const lastNotifiedStatus = useRef<string>('');

  useEffect(() => {
    if (data?.current && notificationPermission === 'granted') {
      const status = data.current.healthStatus;
      if ((status === 'Needs Attention' || status === 'Critical') && lastNotifiedStatus.current !== status) {
        new Notification('PlantSense Alert 🌱', {
          body: `Your Spider Plant is thirsty! Add ${data.current.recommendedWaterML}mL of water.`,
        });
        lastNotifiedStatus.current = status;
      }
    }
  }, [data?.current?.healthStatus, notificationPermission]);

  const requestNotifications = async () => {
    const permission = await Notification.requestPermission();
    setNotificationPermission(permission);
  };

  if (loading) {
    return (
      <div className="flex items-center justify-center h-full min-h-[400px]">
        <div className="animate-pulse flex flex-col items-center">
          <div className="w-16 h-16 border-4 border-primary border-t-transparent rounded-full animate-spin mb-4" />
          <p className="text-gray-500 font-medium">Connecting to PlantSense AI...</p>
        </div>
      </div>
    );
  }

  if (error || !data || !data.current) {
    return (
      <div className="flex items-center justify-center h-full min-h-[400px]">
        <Card className="max-w-md bg-destructive/10 border-destructive/20">
          <CardContent className="flex flex-col items-center pt-6 text-center">
            <AlertCircle className="w-12 h-12 text-destructive mb-4" />
            <h2 className="text-xl font-bold text-destructive mb-2">Sensor Offline</h2>
            <p className="text-sm text-destructive/80">
              Unable to reach the ESP32. Please check its power and WiFi connection.
            </p>
          </CardContent>
        </Card>
      </div>
    );
  }

  const { current } = data;
  const isHealthy = current.healthStatus === 'Excellent' || current.healthStatus === 'Healthy';
  
  return (
    <div className="space-y-6">
      {/* Header section */}
      <div className="flex flex-col md:flex-row justify-between items-start md:items-end gap-4 pb-4">
        <div>
          <h1 className="text-3xl md:text-4xl font-bold tracking-tight mb-2">Spider Plant</h1>
          <p className="text-gray-500 flex items-center gap-2">
            <span className="relative flex h-3 w-3">
              <span className="animate-ping absolute inline-flex h-full w-full rounded-full bg-green-400 opacity-75"></span>
              <span className="relative inline-flex rounded-full h-3 w-3 bg-green-500"></span>
            </span>
            Live Updates • Last sync: {current.timestamp ? formatDistanceToNow(current.timestamp, { addSuffix: true }) : 'Just now'}
          </p>
        </div>
        <div className="flex items-center gap-4">
          {notificationPermission !== 'granted' && (
            <button onClick={requestNotifications} className="text-sm bg-blue-100 text-blue-700 px-3 py-1.5 rounded-full font-medium hover:bg-blue-200 transition-colors">
              Enable Alerts 🔔
            </button>
          )}
          <div className={`px-4 py-2 rounded-full font-semibold flex items-center gap-2 ${
          isHealthy ? 'bg-green-100 text-green-700 dark:bg-green-900/30 dark:text-green-400' 
            : 'bg-orange-100 text-orange-700 dark:bg-orange-900/30 dark:text-orange-400'
        }`}>
          <Activity className="w-4 h-4" />
          {current.healthStatus}
        </div>
      </div>
      </div>

      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
        {/* Main Moisture Card */}
        <Card className="lg:col-span-2 relative overflow-hidden group">
          <div className="absolute inset-0 bg-gradient-to-br from-blue-500/10 to-primary/10 opacity-50 transition-opacity group-hover:opacity-100" />
          <CardHeader>
            <CardTitle className="flex items-center gap-2 text-xl">
              <Droplet className="text-blue-500" /> Soil Moisture
            </CardTitle>
          </CardHeader>
          <CardContent className="flex flex-col md:flex-row items-center justify-between gap-6 relative z-10">
            <div className="relative">
              {/* Circular Progress (CSS based) */}
              <svg className="w-48 h-48 transform -rotate-90">
                <circle cx="96" cy="96" r="88" className="stroke-gray-200 dark:stroke-gray-800" strokeWidth="12" fill="none" />
                <circle cx="96" cy="96" r="88" className="stroke-blue-500 transition-all duration-1000 ease-out" strokeWidth="12" fill="none" strokeDasharray="552.92" strokeDashoffset={552.92 - (552.92 * (current.moisture || 0)) / 100} strokeLinecap="round" />
              </svg>
              <div className="absolute inset-0 flex flex-col items-center justify-center">
                <span className="text-5xl font-bold tracking-tighter">{current.moisture}%</span>
                <span className="text-sm font-medium text-gray-500 uppercase tracking-widest mt-1">Water</span>
              </div>
            </div>
            
            <div className="flex-1 space-y-4">
              <div className="p-4 rounded-xl bg-white/50 dark:bg-black/20 backdrop-blur-md border border-black/5 dark:border-white/5">
                <h4 className="font-semibold text-sm text-gray-500 mb-1 uppercase tracking-wider">AI Recommendation</h4>
                <p className="text-lg font-medium">{current.recommendationText}</p>
              </div>
              
              <div className="grid grid-cols-2 gap-4">
                <div className="p-4 rounded-xl bg-white/50 dark:bg-black/20 backdrop-blur-md border border-black/5 dark:border-white/5">
                  <div className="flex items-center gap-2 text-gray-500 mb-1">
                    <Droplet className="w-4 h-4" /> <span className="text-xs font-semibold uppercase">Add Water</span>
                  </div>
                  <p className="text-2xl font-bold">{current.recommendedWaterML} <span className="text-sm font-normal text-gray-500">mL</span></p>
                </div>
                <div className="p-4 rounded-xl bg-white/50 dark:bg-black/20 backdrop-blur-md border border-black/5 dark:border-white/5">
                  <div className="flex items-center gap-2 text-gray-500 mb-1">
                    <Clock className="w-4 h-4" /> <span className="text-xs font-semibold uppercase">Next Water</span>
                  </div>
                  <p className="text-xl font-bold">~{current.nextWateringEstimateHours} <span className="text-sm font-normal text-gray-500">hrs</span></p>
                </div>
              </div>
            </div>
          </CardContent>
        </Card>

        {/* Environment Cards */}
        <div className="space-y-6">
          <Card className="h-[calc(50%-12px)]">
            <CardHeader className="pb-2">
              <CardTitle className="flex items-center gap-2 text-gray-500 text-sm uppercase tracking-wider">
                <Thermometer className="w-4 h-4 text-orange-500" /> Temperature
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="flex items-baseline gap-2">
                <span className="text-5xl font-bold">{current.temperature?.toFixed(1)}</span>
                <span className="text-xl text-gray-500 font-medium">°C</span>
              </div>
              <p className="text-sm text-gray-500 mt-2">Ideal range: 18-27°C</p>
            </CardContent>
          </Card>

          <Card className="h-[calc(50%-12px)]">
            <CardHeader className="pb-2">
              <CardTitle className="flex items-center gap-2 text-gray-500 text-sm uppercase tracking-wider">
                <Wind className="w-4 h-4 text-teal-500" /> Humidity
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="flex items-baseline gap-2">
                <span className="text-5xl font-bold">{current.humidity?.toFixed(1)}</span>
                <span className="text-xl text-gray-500 font-medium">%</span>
              </div>
              <p className="text-sm text-gray-500 mt-2">Ideal range: 40-70%</p>
            </CardContent>
          </Card>
        </div>
      </div>
    </div>
  );
};

export default Dashboard;
