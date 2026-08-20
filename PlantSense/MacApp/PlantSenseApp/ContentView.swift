import SwiftUI
import Combine

// MARK: - Data Models
struct PlantData: Codable {
    let air_humidity: Double?
    let air_temp: Double?
    let soil_moisture: Int?
    let recommendation: Recommendation?
}

struct Recommendation: Codable {
    let healthStatus: String?
    let recommendedWaterML: Int?
    let nextWateringEstimateHours: Int?
    let recommendationText: String?
}

// MARK: - Data Manager
class PlantDataManager: ObservableObject {
    @Published var data: PlantData?
    @Published var isLoading = true
    @Published var isError = false
    
    private var timer: Timer?
    
    func startFetching() {
        fetchData()
        timer = Timer.scheduledTimer(withTimeInterval: 5.0, repeats: true) { [weak self] _ in
            self?.fetchData()
        }
    }
    
    func stopFetching() {
        timer?.invalidate()
    }
    
    private func fetchData() {
        guard let url = URL(string: "https://agrosense-e00de-default-rtdb.firebaseio.com/sensors/plantsense.json") else { return }
        
        let task = URLSession.shared.dataTask(with: url) { [weak self] data, response, error in
            DispatchQueue.main.async {
                self?.isLoading = false
                if error != nil {
                    self?.isError = true
                    return
                }
                if let data = data {
                    do {
                        self?.data = try JSONDecoder().decode(PlantData.self, from: data)
                        self?.isError = false
                    } catch {
                        self?.isError = true
                        print("JSON Parse Error: \(error)")
                    }
                }
            }
        }
        task.resume()
    }
}

// MARK: - Views
struct ContentView: View {
    @StateObject private var manager = PlantDataManager()
    
    var body: some View {
        ZStack {
            // Dynamic Organic Background
            LinearGradient(
                gradient: Gradient(colors: getBackgroundColors(for: manager.data?.recommendation?.healthStatus)),
                startPoint: .topLeading,
                endPoint: .bottomTrailing
            )
            .ignoresSafeArea()
            
            if manager.isLoading && manager.data == nil {
                VStack(spacing: 20) {
                    ProgressView()
                        .scaleEffect(1.5)
                    Text("Connecting to PlantSense...")
                        .font(.headline)
                        .foregroundColor(.white)
                }
            } else if manager.isError && manager.data == nil {
                ErrorView { manager.startFetching() }
            } else if let data = manager.data {
                ScrollView {
                    DashboardView(data: data)
                        .padding()
                }
            }
        }
        .onAppear {
            manager.startFetching()
        }
    }
    
    func getBackgroundColors(for health: String?) -> [Color] {
        if health == "Critical" || health == "Needs Attention" {
            return [Color(red: 0.3, green: 0.1, blue: 0.1), Color(red: 0.1, green: 0.05, blue: 0.05)]
        } else {
            return [Color(red: 0.05, green: 0.25, blue: 0.15), Color(red: 0.02, green: 0.1, blue: 0.08)]
        }
    }
}

struct DashboardView: View {
    let data: PlantData
    @State private var isAnimating = false
    
    var body: some View {
        VStack(spacing: 24) {
            // Header
            HStack {
                VStack(alignment: .leading, spacing: 4) {
                    Text("Spider Plant")
                        .font(.system(size: 34, weight: .heavy, design: .rounded))
                        .foregroundColor(.white)
                    
                    HStack {
                        Circle()
                            .fill(Color.green)
                            .frame(width: 8, height: 8)
                            .opacity(isAnimating ? 1.0 : 0.4)
                            .animation(.easeInOut(duration: 1.0).repeatForever(), value: isAnimating)
                        Text("Live Sync")
                            .font(.subheadline)
                            .foregroundColor(.white.opacity(0.7))
                    }
                }
                Spacer()
                
                // Status Pill
                let isHealthy = (data.recommendation?.healthStatus == "Healthy")
                HStack {
                    Image(systemName: isHealthy ? "leaf.fill" : "exclamationmark.triangle.fill")
                    Text(data.recommendation?.healthStatus ?? "Analyzing")
                }
                .font(.system(size: 14, weight: .bold))
                .padding(.horizontal, 16)
                .padding(.vertical, 8)
                .background(.ultraThinMaterial)
                .foregroundColor(isHealthy ? .green : .orange)
                .clipShape(Capsule())
                .shadow(color: isHealthy ? .green.opacity(0.3) : .orange.opacity(0.3), radius: 10)
            }
            .padding(.top, 10)
            .onAppear { isAnimating = true }
            
            // Main Metrics Grid
            LazyVGrid(columns: [GridItem(.adaptive(minimum: 160), spacing: 16)], spacing: 16) {
                // Soil Moisture Hero Card
                VStack(spacing: 16) {
                    Text("SOIL MOISTURE")
                        .font(.caption.bold())
                        .foregroundColor(.white.opacity(0.6))
                        .tracking(2)
                    
                    ZStack {
                        Circle()
                            .stroke(Color.white.opacity(0.1), lineWidth: 16)
                        
                        let moisture = Double(data.soil_moisture ?? 0)
                        Circle()
                            .trim(from: 0.0, to: CGFloat(min(max(moisture / 100.0, 0.0), 1.0)))
                            .stroke(
                                AngularGradient(
                                    gradient: Gradient(colors: [.blue, .cyan]),
                                    center: .center,
                                    startAngle: .degrees(0),
                                    endAngle: .degrees(360)
                                ),
                                style: StrokeStyle(lineWidth: 16, lineCap: .round)
                            )
                            .rotationEffect(.degrees(-90))
                            .animation(.spring(response: 1.0, dampingFraction: 0.7), value: moisture)
                        
                        VStack(spacing: 0) {
                            Text("\(Int(moisture))%")
                                .font(.system(size: 36, weight: .bold, design: .rounded))
                                .foregroundColor(.white)
                        }
                    }
                    .frame(width: 120, height: 120)
                    .padding(.vertical, 8)
                }
                .padding(20)
                .frame(maxWidth: .infinity)
                .background(.ultraThinMaterial)
                .clipShape(RoundedRectangle(cornerRadius: 24, style: .continuous))
                
                // Temp & Humidity Side Cards
                VStack(spacing: 16) {
                    MetricCard(
                        title: "TEMPERATURE",
                        value: String(format: "%.1f°", data.air_temp ?? 0.0),
                        icon: "thermometer.sun.fill",
                        color: .orange
                    )
                    
                    MetricCard(
                        title: "HUMIDITY",
                        value: "\(Int(data.air_humidity ?? 0))%",
                        icon: "humidity.fill",
                        color: .teal
                    )
                }
            }
            
            // Gemini AI Insights Card
            VStack(alignment: .leading, spacing: 16) {
                HStack {
                    if #available(macOS 14.0, iOS 17.0, *) {
                        Image(systemName: "sparkles")
                            .foregroundColor(.purple)
                            .symbolEffect(.pulse, options: .repeating)
                    } else {
                        Image(systemName: "sparkles")
                            .foregroundColor(.purple)
                    }
                    Text("Gemini AI Insights")
                        .font(.headline)
                        .foregroundColor(.purple)
                    Spacer()
                }
                
                Text(data.recommendation?.recommendationText ?? "Analyzing current metrics to provide intelligent care recommendations...")
                    .font(.system(.body, design: .rounded))
                    .foregroundColor(.white.opacity(0.9))
                    .lineSpacing(4)
                
                if let status = data.recommendation?.healthStatus,
                   (status == "Needs Attention" || status == "Critical"),
                   let waterML = data.recommendation?.recommendedWaterML, waterML > 0 {
                    HStack {
                        Image(systemName: "drop.triangle.fill")
                        Text("ACTION REQUIRED: ADD \(waterML)mL OF WATER")
                            .font(.subheadline.bold())
                    }
                    .padding()
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .background(Color.red.opacity(0.2))
                    .foregroundColor(.red)
                    .clipShape(RoundedRectangle(cornerRadius: 12, style: .continuous))
                    .overlay(
                        RoundedRectangle(cornerRadius: 12)
                            .stroke(Color.red.opacity(0.5), lineWidth: 1)
                    )
                    .padding(.top, 8)
                }
            }
            .padding(24)
            .frame(maxWidth: .infinity)
            .background(.ultraThinMaterial)
            .clipShape(RoundedRectangle(cornerRadius: 24, style: .continuous))
            .overlay(
                RoundedRectangle(cornerRadius: 24)
                    .stroke(LinearGradient(colors: [.purple.opacity(0.5), .clear], startPoint: .topLeading, endPoint: .bottomTrailing), lineWidth: 1)
            )
            
            Spacer(minLength: 40)
        }
    }
}

struct MetricCard: View {
    let title: String
    let value: String
    let icon: String
    let color: Color
    
    var body: some View {
        HStack {
            VStack(alignment: .leading, spacing: 6) {
                Text(title)
                    .font(.caption2.bold())
                    .foregroundColor(.white.opacity(0.6))
                    .tracking(1)
                
                Text(value)
                    .font(.system(size: 28, weight: .bold, design: .rounded))
                    .foregroundColor(.white)
            }
            Spacer()
            
            ZStack {
                Circle()
                    .fill(color.opacity(0.2))
                    .frame(width: 44, height: 44)
                Image(systemName: icon)
                    .font(.title3)
                    .foregroundColor(color)
            }
        }
        .padding(20)
        .frame(maxWidth: .infinity)
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 24, style: .continuous))
    }
}

struct ErrorView: View {
    let retryAction: () -> Void
    
    var body: some View {
        VStack(spacing: 16) {
            Image(systemName: "wifi.exclamationmark")
                .font(.system(size: 50))
                .foregroundColor(.red)
            Text("Connection Failed")
                .font(.title2.bold())
                .foregroundColor(.white)
            Text("Unable to reach Firebase.")
                .foregroundColor(.white.opacity(0.7))
            
            Button(action: retryAction) {
                Text("Retry Connection")
                    .bold()
                    .padding(.horizontal, 24)
                    .padding(.vertical, 12)
                    .background(Color.white)
                    .foregroundColor(.black)
                    .clipShape(Capsule())
            }
            .padding(.top, 8)
        }
        .padding(40)
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 24, style: .continuous))
    }
}
