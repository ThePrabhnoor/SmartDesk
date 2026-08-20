import React from 'react';
import { NavLink, Outlet } from 'react-router-dom';
import { Leaf, BarChart2, Settings, Info } from 'lucide-react';
import { cn } from '../../lib/utils';
import { motion } from 'framer-motion';

export const Layout = () => {
  const navItems = [
    { name: 'Dashboard', path: '/', icon: Leaf },
    { name: 'Analytics', path: '/analytics', icon: BarChart2 },
    { name: 'Settings', path: '/settings', icon: Settings },
    { name: 'About', path: '/about', icon: Info },
  ];

  return (
    <div className="min-h-screen bg-gray-50 dark:bg-black text-gray-900 dark:text-gray-100 flex overflow-hidden transition-colors duration-300">
      {/* Background gradients for that Apple/Premium feel */}
      <div className="fixed inset-0 z-0 pointer-events-none">
        <div className="absolute top-[-10%] left-[-10%] w-[40%] h-[40%] rounded-full bg-primary/20 blur-[120px] mix-blend-screen" />
        <div className="absolute bottom-[-10%] right-[-10%] w-[50%] h-[50%] rounded-full bg-blue-500/10 blur-[120px] mix-blend-screen" />
      </div>

      {/* Sidebar */}
      <aside className="relative z-10 w-64 glass-panel border-r border-white/20 dark:border-white/10 hidden md:flex flex-col">
        <div className="p-6 flex items-center space-x-3">
          <div className="w-10 h-10 rounded-xl bg-gradient-to-br from-primary to-green-600 flex items-center justify-center shadow-lg">
            <Leaf className="text-white w-6 h-6" />
          </div>
          <h1 className="text-xl font-bold tracking-tight">PlantSense</h1>
        </div>
        
        <nav className="flex-1 px-4 space-y-2 mt-4">
          {navItems.map((item) => (
            <NavLink
              key={item.name}
              to={item.path}
              className={({ isActive }) =>
                cn(
                  'flex items-center space-x-3 px-4 py-3 rounded-xl transition-all duration-200 group relative overflow-hidden',
                  isActive 
                    ? 'text-white bg-primary/90 shadow-md' 
                    : 'text-gray-600 dark:text-gray-400 hover:bg-gray-100 dark:hover:bg-white/5 hover:text-gray-900 dark:hover:text-gray-100'
                )
              }
            >
              <item.icon className="w-5 h-5 z-10 relative" />
              <span className="font-medium z-10 relative">{item.name}</span>
            </NavLink>
          ))}
        </nav>
      </aside>

      {/* Main Content */}
      <main className="flex-1 relative z-10 overflow-y-auto">
        <motion.div
          initial={{ opacity: 0, y: 10 }}
          animate={{ opacity: 1, y: 0 }}
          exit={{ opacity: 0, y: -10 }}
          transition={{ duration: 0.3 }}
          className="p-6 md:p-10 max-w-7xl mx-auto min-h-full"
        >
          <Outlet />
        </motion.div>
      </main>
      
      {/* Mobile Nav (Bottom Bar) */}
      <nav className="md:hidden fixed bottom-0 left-0 right-0 z-20 glass-panel border-t border-white/20 pb-safe">
        <div className="flex justify-around p-3">
          {navItems.map((item) => (
            <NavLink
              key={item.name}
              to={item.path}
              className={({ isActive }) =>
                cn(
                  'flex flex-col items-center p-2 rounded-xl transition-colors',
                  isActive ? 'text-primary' : 'text-gray-500'
                )
              }
            >
              <item.icon className="w-6 h-6 mb-1" />
              <span className="text-[10px] font-medium">{item.name}</span>
            </NavLink>
          ))}
        </div>
      </nav>
    </div>
  );
};
