#pragma once
// Mk::wg2 国际化：key → 文本表；未命中回退 key 本身
#include <initializer_list>
#include <string>
#include <unordered_map>

namespace Mk::wg2 {

inline std::unordered_map<std::string, std::string>& translations() {
	static std::unordered_map<std::string, std::string> table;
	return table;
}

inline void clearTranslations() { translations().clear(); }

inline void setTranslation(const char* key, const char* text) {
	if (!key) return;
	translations()[key] = text ? text : "";
}

inline void setTranslations(std::initializer_list<std::pair<const char*, const char*>> list) {
	for (const auto& kv : list) setTranslation(kv.first, kv.second);
}

inline const char* tr(const char* key) {
	if (!key) return "";
	const auto& table = translations();
	const auto it = table.find(key);
	return it == table.end() ? key : it->second.c_str();
}

inline std::string trStr(const std::string& key) { return tr(key.c_str()); }

} // namespace Mk::wg2
