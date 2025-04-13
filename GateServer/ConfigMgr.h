#pragma once
#include <map>
#include <string>

struct SectionInfo {
	SectionInfo() {};
	~SectionInfo(){ _section_datas.clear(); }

	// ���ƹ��캯��������ʹ��Ĭ�ϵĿ������캯��
	SectionInfo(const SectionInfo& other) {
		_section_datas = other._section_datas;
	}

	// ����=�����������ʹ��Ĭ�ϵĸ�ֵ���������ֹ�Լ������Լ�
	SectionInfo& operator=(const SectionInfo& other) {
		if (this != &other) {
			_section_datas = other._section_datas;
		}
		return *this;
	}

	std::map<std::string, std::string> _section_datas; // key��value��map
	// ����[]�����������ʹ�ã�����ʹ��find�����µ��鷳����������ֵ��
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

	// �������캯��
	ConfigMgr(const ConfigMgr& other) {
		_config_map = other._config_map;
	}
	// ������ֵ����������=�����
	ConfigMgr& operator=(const ConfigMgr& other) {
		if (this != &other) {
			_config_map = other._config_map;
		}
		return *this;
	}
	
private:
	std::map<std::string, SectionInfo> _config_map; // section��map��map
};

