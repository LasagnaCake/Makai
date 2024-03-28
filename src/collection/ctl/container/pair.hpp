#ifndef CTL_CONTAINER_PAIR_H
#define CTL_CONTAINER_PAIR_H

template<class K, class V>
class Pair {
public:
	typedef K KeyType;
	typedef V ValueType;

	KeyType&	key		= left;
	ValueType&	value	= right;

	union {KeyType left, first, a;		};
	union {KeyType right, second, b;	};
};

#endif // CTL_CONTAINER_PAIR_H