import WidgetKit
import SwiftUI

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
    let recommendationText: String?
}

// MARK: - Timeline Provider
struct Provider: TimelineProvider {
    func placeholder(in context: Context) -> SimpleEntry {
        SimpleEntry(date: Date(), data: nil, error: false)
    }

    func getSnapshot(in context: Context, completion: @escaping (SimpleEntry) -> ()) {
        let sampleData = PlantData(air_humidity: 50, air_temp: 24.5, soil_moisture: 75, recommendation: Recommendation(healthStatus: "Healthy", recommendedWaterML: 0, recommendationText: "Doing great!"))
        let entry = SimpleEntry(date: Date(), data: sampleData, error: false)
        completion(entry)
    }

    func getTimeline(in context: Context, completion: @escaping (Timeline<Entry>) -> ()) {
        // Prevent WidgetKit Simulator crashes by returning mock data in previews
        if context.isPreview {
            let sampleData = PlantData(air_humidity: 50, air_temp: 24.5, soil_moisture: 75, recommendation: Recommendation(healthStatus: "Healthy", recommendedWaterML: 0, recommendationText: "Doing great!"))
            let entry = SimpleEntry(date: Date(), data: sampleData, error: false)
            let timeline = Timeline(entries: [entry], policy: .atEnd)
            completion(timeline)
            return
        }

        let url = URL(string: "https://agrosense-e00de-default-rtdb.firebaseio.com/sensors/plantsense.json")!
        
        let task = URLSession.shared.dataTask(with: url) { data, response, error in
            var plantData: PlantData? = nil
            var hasError = error != nil
            
            if let data = data {
                do {
                    plantData = try JSONDecoder().decode(PlantData.self, from: data)
                } catch {
                    hasError = true
                }
            }
            
            let entry = SimpleEntry(date: Date(), data: plantData, error: hasError)
            
            // Refresh every 15 minutes to save battery
            let nextUpdateDate = Calendar.current.date(byAdding: .minute, value: 15, to: Date())!
            let timeline = Timeline(entries: [entry], policy: .after(nextUpdateDate))
            completion(timeline)
        }
        task.resume()
    }
}

struct SimpleEntry: TimelineEntry {
    let date: Date
    let data: PlantData?
    let error: Bool
}

// MARK: - Widget Views
struct PlantSenseWidgetEntryView : View {
    var entry: Provider.Entry
    @Environment(\.widgetFamily) var family

    var body: some View {
        Group {
            if entry.error {
                VStack {
                    Image(systemName: "wifi.exclamationmark")
                        .foregroundColor(.red)
                        .font(.title2)
                    Text("Connection Error")
                        .font(.caption)
                }
            } else if let data = entry.data {
                if family == .systemSmall {
                    SmallWidgetView(data: data)
                } else {
                    MediumWidgetView(data: data)
                }
            } else {
                VStack {
                    ProgressView()
                    Text("Syncing...")
                        .font(.caption)
                        .foregroundColor(.secondary)
                }
            }
        }
    }
}

struct SmallWidgetView: View {
    let data: PlantData
    
    var body: some View {
        VStack(spacing: 8) {
            HStack {
                Image(systemName: "leaf.fill")
                    .foregroundColor(data.recommendation?.healthStatus == "Healthy" ? .green : .orange)
                Text("PlantSense")
                    .font(.caption2.bold())
                    .foregroundColor(.primary)
            }
            
            let moisture = Double(data.soil_moisture ?? 0)
            ZStack {
                Circle()
                    .stroke(Color.primary.opacity(0.1), lineWidth: 8)
                
                Circle()
                    .trim(from: 0.0, to: CGFloat(min(max(moisture / 100.0, 0.0), 1.0)))
                    .stroke(
                        AngularGradient(colors: [.blue, .cyan], center: .center),
                        style: StrokeStyle(lineWidth: 8, lineCap: .round)
                    )
                    .rotationEffect(.degrees(-90))
                
                VStack(spacing: -2) {
                    Text("\(Int(moisture))%")
                        .font(.system(.title2, design: .rounded).bold())
                    Text("WATER")
                        .font(.system(size: 8, weight: .bold))
                        .foregroundColor(.secondary)
                }
            }
            .frame(width: 80, height: 80)
        }
        .padding()
    }
}

struct MediumWidgetView: View {
    let data: PlantData
    
    var body: some View {
        HStack(spacing: 16) {
            // Left Side: Moisture
            let moisture = Double(data.soil_moisture ?? 0)
            ZStack {
                Circle()
                    .stroke(Color.primary.opacity(0.1), lineWidth: 10)
                
                Circle()
                    .trim(from: 0.0, to: CGFloat(min(max(moisture / 100.0, 0.0), 1.0)))
                    .stroke(
                        AngularGradient(colors: [.blue, .cyan], center: .center),
                        style: StrokeStyle(lineWidth: 10, lineCap: .round)
                    )
                    .rotationEffect(.degrees(-90))
                
                VStack(spacing: -2) {
                    Text("\(Int(moisture))%")
                        .font(.system(.title, design: .rounded).bold())
                    Text("WATER")
                        .font(.system(size: 10, weight: .bold))
                        .foregroundColor(.secondary)
                }
            }
            .frame(width: 100, height: 100)
            
            Divider()
            
            // Right Side: Details
            VStack(alignment: .leading, spacing: 8) {
                HStack {
                    Image(systemName: "leaf.fill")
                        .foregroundColor(data.recommendation?.healthStatus == "Healthy" ? .green : .orange)
                    Text("Spider Plant")
                        .font(.headline)
                }
                
                HStack(spacing: 12) {
                    HStack(spacing: 4) {
                        Image(systemName: "thermometer.sun.fill")
                            .foregroundColor(.orange)
                        Text(String(format: "%.1f°", data.air_temp ?? 0.0))
                    }
                    HStack(spacing: 4) {
                        Image(systemName: "humidity.fill")
                            .foregroundColor(.teal)
                        Text("\(Int(data.air_humidity ?? 0))%")
                    }
                }
                .font(.caption.bold())
                
                if let status = data.recommendation?.healthStatus,
                   status != "Healthy",
                   let waterML = data.recommendation?.recommendedWaterML, waterML > 0 {
                    Text("ACTION: Add \(waterML)mL water")
                        .font(.caption2.bold())
                        .foregroundColor(.red)
                        .padding(4)
                        .background(Color.red.opacity(0.1))
                        .cornerRadius(4)
                } else {
                    Text("Gemini: \(data.recommendation?.healthStatus ?? "Healthy")")
                        .font(.caption2)
                        .foregroundColor(.secondary)
                }
            }
            Spacer(minLength: 0)
        }
        .padding()
    }
}

// MARK: - Widget Configuration
struct PlantSenseWidget: Widget {
    let kind: String = "PlantSenseWidget"

    var body: some WidgetConfiguration {
        StaticConfiguration(kind: kind, provider: Provider()) { entry in
            PlantSenseWidgetEntryView(entry: entry)
                .containerBackground(.fill.tertiary, for: .widget)
        }
        .configurationDisplayName("PlantSense")
        .description("Keep track of your plant's health and water needs.")
        .supportedFamilies([.systemSmall, .systemMedium])
    }
}
