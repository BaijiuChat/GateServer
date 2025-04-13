#include "ConfigMgr.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

ConfigMgr::ConfigMgr() {
	boost::filesystem::path config_path = boost::filesystem::current_path() / "config.ini";
	std::cout << "config_path is " << config_path.string() << std::endl;

	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(config_path.string(), pt);

	for (const auto& [section_name, section_tree] : pt) {
		// ֱ���� map �й��� SectionInfo
		auto& section_info = _config_map[section_name];
		// ֱ�ӱ�������� section_datas
		for (const auto& [key, value_node] : section_tree) {
			section_info._section_datas.emplace(key, value_node.data());
		}
	}

	// ����̨��������ļ�����
	for (const auto& [section_name, section_info] : _config_map) {
		std::cout << "Section: " << section_name << std::endl;
		for (const auto& [key, value] : section_info._section_datas) {
			std::cout << "  " << key << ": " << value << std::endl;
		}
	}
}

/////////////// �ɰ���� ///////////////
//boost::property_tree::ptree pt; // ����һ��ptree����
//boost::property_tree::read_ini(config_path.string(), pt); // ��ȡ�����ļ�
//
//for (const auto& section_pair : pt) {
//	const std::string& section_name = section_pair.first; // ��ȡsection������
//
//	const boost::property_tree::ptree& section_tree = section_pair.second; // ��ȡsection������
//	std::map<std::string, std::string> section_config; // ����һ��map���洢�ڲ�section������
//	for (const auto& item : section_tree) {
//		const std::string& key = item.first; // ��ȡkey
//		const std::string& value = item.second.data(); // ��ȡvalue
//		section_config[key] = value; // ��key��value����map��
//	}
//	SectionInfo section_info; // ����һ��SectionInfo����
//	section_info._section_datas = section_config; // ��map��ֵ��SectionInfo����
//
//	_config_map[section_name] = section_info; // ��SectionInfo�������_config_map��
//}
/////////////// �ɰ���� ///////////////