#undef DEBUG
#define DEBUG(...)		Makai::Fold::strins(std::cout, __VA_ARGS__)
#undef DEBUGLN
#define DEBUGLN(...)	DEBUG( __VA_ARGS__, "\n")
