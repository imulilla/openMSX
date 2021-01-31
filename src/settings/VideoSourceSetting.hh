#ifndef VIDEOSOURCESETTING_HH
#define VIDEOSOURCESETTING_HH

#include "Setting.hh"
#include <vector>
#include <utility>

namespace openmsx {

class VideoSourceSetting final : public Setting
{
public:
	explicit VideoSourceSetting(CommandController& commandController);

	[[nodiscard]] std::string_view getTypeString() const override;
	void additionalInfo(TclObject& result) const override;
	void tabCompletion(std::vector<std::string>& tokens) const override;

	[[nodiscard]] int registerVideoSource(const std::string& source);
	void unregisterVideoSource(int source);
	[[nodiscard]] int getSource() noexcept;
	void setSource(int id);

private:
	[[nodiscard]] std::vector<std::string_view> getPossibleValues() const;
	void checkSetValue(std::string_view value) const;
	[[nodiscard]] bool has(int value) const;
	[[nodiscard]] int has(std::string_view value) const;

private:
	std::vector<std::pair<std::string, int>> sources; // unordered
};

class VideoSourceActivator
{
public:
	VideoSourceActivator(
		VideoSourceSetting& setting, const std::string& name);
	~VideoSourceActivator();
	[[nodiscard]] int getID() const { return id; }

private:
	VideoSourceSetting& setting;
	int id;
};

} // namespace openmsx

#endif
