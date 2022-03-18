#pragma once
#include <string>

namespace qian {

class Timestamp{
public:
	Timestamp();
	explicit Timestamp(int64_t microseconds);
	static Timestamp now();
	std::string toString() const;
private:
	int64_t microseconds_;
};

} // namespace qian