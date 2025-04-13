#pragma once
#include <map>
#include <string>

struct SectionInfo {
	SectionInfo() {};
	~SectionInfo(){ _section_datas.clear(); }

	// 复制构造函数，避免使用默认的拷贝构造函数
	SectionInfo(const SectionInfo& other) {
		_section_datas = other._section_datas;
	}

	// 重载=运算符，避免使用默认的赋值运算符，禁止自己拷贝自己
	SectionInfo& operator=(const SectionInfo& other) {
		if (this != &other) {
			_section_datas = other._section_datas;
		}
		return *this;
	}

	std::map<std::string, std::string> _section_datas; // key和value的map
	// 重载[]运算符，方便使用，避免使用find而导致的麻烦，比如插入空值等
	std::string operator[](const std::string& key) const {
		auto it = _section_datas.find(key);
		if (it != _section_datas.end()) {
			return it->second;
		}
		return "";
	}
};;
class ConfigMgr
{
public:
	ConfigMgr();
	~ConfigMgr()
	{
		_config_map.clear();
	}
	
	SectionInfo operator[](const std::string& section) {
		if (_config_map.find(section) == _config_map.end()) {
			return SectionInfo();
		}
		return _config_map[section];
	}

	// 拷贝构造函数
	ConfigMgr(const ConfigMgr& other) {
		_config_map = other._config_map;
	}
	// 拷贝赋值函数，重载=运算符
	ConfigMgr& operator=(const ConfigMgr& other) {
		if (this != &other) {
			_config_map = other._config_map;
		}
		return *this;
	}
	
private:
	std::map<std::string, SectionInfo> _config_map; // section和map的map
};

