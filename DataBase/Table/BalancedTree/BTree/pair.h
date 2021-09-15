#ifndef PAIR_H
#define PAIR_H

template <typename K, typename V>
class Pair
{
public:
    K first;
    V second;

    Pair(K key, V value):first(key), second(value) {}

    bool operator==(const Pair<K, V> other) { return first == other.first; }
    bool operator!=(const Pair<K, V> other) { return first != other.first; }
    bool operator>(const Pair<K, V> other) { return first > other.first; }
    bool operator>=(const Pair<K, V> other) { return first >= other.first; }
    bool operator<(const Pair<K, V> other) { return first < other.first; }
    bool operator<=(const Pair<K, V> other) { return first <= other.first; }
};

#endif // PAIR_H
