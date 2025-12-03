#include "BackgroundComponent.hpp"

// If using nlohmann::json helpers:
void to_json(nlohmann::json& j, const BackgroundComponent& c) {
	// ...existing code...
	j["image"] = c.image;
	// ...existing code...
}
void from_json(const nlohmann::json& j, BackgroundComponent& c) {
	// ...existing code...
	if (j.contains("image")) c.image = j.value("image", "");
	// ...existing code...
}
// Or adapt to your engine's serialization system accordingly.