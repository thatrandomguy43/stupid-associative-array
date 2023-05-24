#include <vector>
#include <utility>
#include <algorithm>
#include <optional>
#include <concepts>
#include <variant>

const int EXPANSION_THRESHOLD = 4;
const int ITEMS_PER_NEW_BUCKET = 2;
const int CONTRACTION_THRESHOLD = 1;

template <std::totally_ordered key_t, typename value_t> class StupidCompTable
{
private:
    //templates woooooo
    class Bucket {
    public:
        key_t sort_value;
        std::vector<std::pair<key_t, value_t>> contents;

        std::optional<size_t> search_contents(key_t to_find) {
            if (this->contents.size() == 0) {
                return std::nullopt;
            }
            size_t range_begin = 0;
            size_t range_end = this->contents.size() - 1;
            while (true) {
                size_t pivot = (range_begin + range_end) / 2;
                if (this->contents.at(pivot).first == to_find) {
                    return pivot;
                }
                else if (this->contents.at(pivot).first > to_find) {
                    range_end = pivot;
                }
                else {
                    range_begin = pivot;
                }
                //this situation means the element was not found, as the range is here between 2 elements smaller and larger than the searched key
                if ((range_end - range_begin == 1) and (this->contents.at(range_begin).first < to_find) and (this->contents.at(range_end).first > to_find)){
                    return std::nullopt;
                }
                //when there is only 1 element this will be tripped after checking that 1 element
                if (range_begin == range_end){
                    return std::nullopt;

                }
            }

        }
    };
    std::vector<Bucket> ordered_buckets{};
    size_t item_count = 0;

    void reallocate() {
        size_t new_bucket_count = this->item_count / ITEMS_PER_NEW_BUCKET;
        std::vector<std::pair<key_t,value_t>> serialized_contents{};
        serialized_contents.reserve(item_count);
        for (const auto& buc : this->ordered_buckets){
            for (const auto& item : buc.contents){
                serialized_contents.push_back(item);
            }
        }
        this->ordered_buckets.clear();
        this->ordered_buckets.resize(new_bucket_count);
        size_t item_idx = 0;
        for (size_t bucket_idx = 0; bucket_idx != new_bucket_count; bucket_idx++){
            this->ordered_buckets[bucket_idx].sort_value = serialized_contents[item_idx].first;
            for (; item_idx != ( ITEMS_PER_NEW_BUCKET * (bucket_idx + 1)); item_idx++){
                this->ordered_buckets[bucket_idx].contents.push_back(serialized_contents[item_idx]);
            }
        }
    }

    Bucket& search_buckets(key_t to_find) {
        size_t range_begin = 0;
        size_t range_end = this->ordered_buckets.size() - 1;
        while (range_begin != range_end) {
            size_t pivot = (range_begin + range_end) / 2;
            /*all buckets larger than to_find will never be the answer and thus should be cut out, but the searched bucket should still be included if to_find is larger. 
            buckets store every value larger than their sort but smaller than the next bucket's sort.
            this also has the helpful side effect of finding the first bucket if the value is smaller than the second bucket, making it catch all things below bucket 2
            */
            if (this->ordered_buckets[pivot].sort_value > to_find) {
                range_end = pivot - 1;
            }
            else {
                range_begin = pivot;
            }
        }
        return this->ordered_buckets.at(range_begin);
    }

    size_t insert(std::pair<key_t, value_t> new_value, Bucket& target_bucket) {
        
        
        size_t insert_pos = 0;
        //inserting at the position after the end is ok, because it will become the new element at the end and expand the vector by one, like push_back
        while ((target_bucket.contents.size() > insert_pos) and target_bucket.contents.at(insert_pos) < new_value) {
            insert_pos++;
        }
        //this kinda slow linear time insert should be ok as long as the buckets don't become wildly out of balance
        target_bucket.contents.insert(target_bucket.contents.begin() + insert_pos, new_value);
        return insert_pos;
    }

public:

    StupidCompTable() {
        this->ordered_buckets.push_back(Bucket{});
    }
    //TODO replace return value with an iterator when i get around to those. this is a little clunky but ok for now. i really don't want to return a dangerous ptr, so nullptr_t makes "no result" explicit.
    std::variant<std::pair<key_t, value_t>*, nullptr_t> find(key_t lookup_key){
        Bucket& target_bucket = this->search_buckets(lookup_key);
        std::optional<size_t> element_pos = target_bucket.search_contents(lookup_key);
        if (element_pos.has_value()){
            return &target_bucket.contents.at(element_pos.value());
        } else {
            return nullptr;
        }
    }

    void erase(key_t delete_key){
        Bucket& target_bucket = this->search_buckets(delete_key);
        std::optional<size_t> element_pos = target_bucket.search_contents(delete_key);
        if (not element_pos.has_value()) {
            return;
        }
        target_bucket.contents.erase(target_bucket.contents.begin() + element_pos);
        this->item_count--;
        if (this->item_count <= this->ordered_buckets.size() * CONTRACTION_THRESHOLD){
            this->reallocate();
        }
    }

    value_t& set(key_t lookup_key) {
        Bucket& target_bucket = this->search_buckets(lookup_key);
        std::optional<size_t> element_pos = target_bucket.search_contents(lookup_key);
        if (not element_pos.has_value()) {
            element_pos = this->insert({ lookup_key,value_t{} }, target_bucket);
        }
        this->item_count++;
        if (this->item_count >= this->ordered_buckets.size() * EXPANSION_THRESHOLD){
            this->reallocate();
        }
        return target_bucket.contents.at(element_pos.value()).second;
    }

};
