#include "ini_file.hpp"
#include "algorithm.hpp"
#include <fstream>

namespace reshade
{
	namespace
	{
		inline void trim(std::string &str, const char *chars = " \t")
		{
			str.erase(0, str.find_first_not_of(chars));
			str.erase(str.find_last_not_of(chars) + 1);
		}
		inline std::string trim(const std::string &str, const char *chars = " \t")
		{
			std::string res(str);
			trim(res, chars);
			return res;
		}
	}

	ini_file::ini_file(const filesystem::path &path) : _path(path)
	{
		load();
	}
	ini_file::~ini_file()
	{
		save();
	}

	void ini_file::load()
	{
		std::string line, section;
		std::ifstream file(_path.wstring());

		while (std::getline(file, line))
		{
			trim(line);

			if (line.empty() || line[0] == ';' || line[0] == '/')
			{
				continue;
			}

			// Read section name
			if (line[0] == '[')
			{
				section = trim(line.substr(0, line.find(']')), " \t[]");
				continue;
			}

			// Read section content
			const auto assign_index = line.find('=');

			if (assign_index != std::string::npos)
			{
				const auto key = trim(line.substr(0, assign_index));
				const auto value = trim(line.substr(assign_index + 1));

				_sections[section][key] = stdext::split(value, ',');
			}
			else
			{
				_sections[section][line] = 0;
			}
		}
	}
	void ini_file::save() const
	{
		if (!_modified)
		{
			return;
		}

		std::ofstream file(_path.wstring());

		for (const auto &section : _sections)
		{
			file << '[' << section.first << ']' << std::endl;

			for (const auto &section_line : section.second)
			{
				file << section_line.first << '=';

				size_t i = 0;

				for (const auto &item : section_line.second.data())
				{
					if (i++ != 0)
					{
						file << ',';
					}

					file << item;
				}

				file << std::endl;
			}

			file << std::endl;
		}
	}

	variant ini_file::get(const std::string &section, const std::string &key, const variant &default) const
	{
		const auto it1 = _sections.find(section);

		if (it1 == _sections.end())
		{
			return default;
		}

		const auto it2 = it1->second.find(key);

		if (it2 == it1->second.end())
		{
			return default;
		}

		return it2->second;
	}
	void ini_file::set(const std::string &section, const std::string &key, const variant &value)
	{
		_modified = true;
		_sections[section][key] = value;
	}
}
