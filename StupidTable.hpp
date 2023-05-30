#include <vector>
#include <utility>
#include <algorithm>
#include <optional>
#include <concepts>
#include <variant>
#include <stdexcept>

const int EXPANSION_THRESHOLD = 4;
const int ITEMS_PER_NEW_BUCKET = 2;
const int CONTRACTION_THRESHOLD = 1;



template <std::totally_ordered key_t, std::default_initializable value_t> class StupidCompTable
{
private:
    //templates woooooo
    class Bucket {
    public:
        key_t sort_value;
        std::vector<std::pair<key_t, value_t>> contents;

        std::optional<size_t> search_contents(key_t to_find) const {
            if (this->contents.size() == 0) {
                return std::nullopt;
            }
            if (this->contents.size() == 1) {
                if (this->contents.at(0).first == to_find) {
                    return 0;
                } else {
                    return std::nullopt;
                }
            }
            size_t range_begin = 0;
            size_t range_end = this->contents.size() - 1;
            while (true) {
                size_t pivot = (range_begin + range_end) / 2;
                if (this->contents.at(pivot).first > to_find) {
                    range_end = pivot;
                }
                else {
                    range_begin = pivot;
                }
                //once there are only two things left binary search dosent really work anymore, so you just gotta check both things
                if (range_end - range_begin == 1 or range_end - range_begin == 0) {
                    if (this->contents.at(range_begin).first == to_find) {
                        return range_begin;
                    } else if (this->contents.at(range_end).first == to_find) {
                        return range_end;
                    } else {
                        return std::nullopt;
                    }
                }
                //when there is only 1 element this will be tripped after checking that 1 element
            }

        }
    size_t insert(std::pair<key_t, value_t> new_value, size_t& owner_item_count) {
        
        
        size_t insert_pos = 0;
        //inserting at the position after the end is ok, because it will become the new element at the end and expand the vector by one, like push_back
        while ((this->contents.size() > insert_pos) and this->contents.at(insert_pos) < new_value) {
            insert_pos++;
        }
        //this kinda slow linear time insert should be ok as long as the buckets don't become wildly out of balance
        this->contents.insert(this->contents.begin() + insert_pos, new_value);
        owner_item_count++;
        return insert_pos;
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

    size_t search_buckets(key_t to_find) const {
        size_t range_begin = 0;
        size_t range_end = this->ordered_buckets.size() - 1;
        while (range_begin != range_end) {
            size_t pivot = ((range_begin + range_end) / 2) + ((range_begin + range_end) % 2);
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
        return range_begin;
    }





public:

    class const_iterator {
    private:
        const StupidCompTable<key_t,value_t>* const owner;
        bool out_of_bounds;
        key_t key;

    public:
        const_iterator(const StupidCompTable<key_t,value_t>* const owner, bool out_of_bounds, key_t key) : owner(owner), out_of_bounds(out_of_bounds), key(key) {
        }

        void operator++(){
            this->operator++(0);
        }

        void operator--(){
            this->operator--(0);
        }

        void operator++(int){
            size_t current_key_bucket_idx = this->owner->search_buckets(key);
            std::optional<size_t> possibly_removed_key_idx = this->owner->ordered_buckets.at(current_key_bucket_idx).search_contents(key);
            if (not possibly_removed_key_idx.has_value()){
                this->out_of_bounds = true;
                return;
            }
            size_t current_key_idx = possibly_removed_key_idx.value();
            if (current_key_idx == this->owner->ordered_buckets.at(current_key_bucket_idx).contents.size() - 1){
                do {
                    current_key_bucket_idx++;
                    if (current_key_bucket_idx == this->owner->ordered_buckets.size()){
                        out_of_bounds = true;
                        break;
                    }
                } while (this->owner->ordered_buckets.at(current_key_bucket_idx).contents.size() == 0);
                if (not out_of_bounds) {
                    current_key_idx = 0;
                }
            } else {
                current_key_idx++;
            }
            if (not out_of_bounds) {
                key = this->owner->ordered_buckets.at(current_key_bucket_idx).contents.at(current_key_idx).first;
            }
        }

        void operator--(int){
            size_t current_key_bucket_idx = this->owner->search_buckets(key);
            std::optional<size_t> possibly_removed_key_idx = this->owner->ordered_buckets.at(current_key_bucket_idx).search_contents(key);
            if (not possibly_removed_key_idx.has_value()){
                this->out_of_bounds = true;
                return;
            }
            size_t current_key_idx = possibly_removed_key_idx.value();
            if (current_key_idx == 0){
                do {
                    current_key_bucket_idx--;
                    if (current_key_bucket_idx == SIZE_MAX){
                        out_of_bounds = true;
                        break;
                    }
                } while (this->owner->ordered_buckets.at(current_key_bucket_idx).contents.size() == 0);
                if (not out_of_bounds) {
                    current_key_idx = this->owner->ordered_buckets.at(current_key_bucket_idx).contents.size() - 1;
                }
            } else {
                current_key_idx--;
            }
            if (not out_of_bounds) {
                key = this->owner->ordered_buckets.at(current_key_bucket_idx).contents.at(current_key_idx).first;
            }
        }
        bool is_in_bounds() const {
            return not this->out_of_bounds;
        }

        const std::pair<key_t,value_t>& operator*() const {
            size_t key_bucket_idx = this->owner->search_buckets(key);
            std::optional<size_t> possibly_removed_key_idx = this->owner->ordered_buckets.at(key_bucket_idx).search_contents(key);
            if (not possibly_removed_key_idx.has_value()) {
                throw std::out_of_range("Attempted to dereference invalid iterator");
            } else {
                return owner->ordered_buckets.at(key_bucket_idx).contents.at(possibly_removed_key_idx.value());
            }
        }

    };

    class iterator {
    private:
        StupidCompTable<key_t,value_t>* const owner;
        bool out_of_bounds;
        key_t key;

    public:
        iterator(StupidCompTable<key_t,value_t>* const owner, bool out_of_bounds, key_t key) : owner(owner), out_of_bounds(out_of_bounds), key(key) {
        }

        void operator++(){
            this->operator++(0);
        }

        void operator--(){
            this->operator--(0);
        }

        void operator++(int){
            size_t current_key_bucket_idx = this->owner->search_buckets(key);
            std::optional<size_t> possibly_removed_key_idx = this->owner->ordered_buckets.at(current_key_bucket_idx).search_contents(key);
            if (not possibly_removed_key_idx.has_value()){
                this->out_of_bounds = true;
                return;
            }
            size_t current_key_idx = possibly_removed_key_idx.value();
            if (current_key_idx == this->owner->ordered_buckets.at(current_key_bucket_idx).contents.size() - 1){
                do {
                    current_key_bucket_idx++;
                    if (current_key_bucket_idx == this->owner->ordered_buckets.size()){
                        out_of_bounds = true;
                        break;
                    }
                } while (this->owner->ordered_buckets.at(current_key_bucket_idx).contents.size() == 0);
                if (not out_of_bounds) {
                    current_key_idx = 0;
                }
            } else {
                current_key_idx++;
            }
            if (not out_of_bounds) {
                key = this->owner->ordered_buckets.at(current_key_bucket_idx).contents.at(current_key_idx).first;
            }
        }

        void operator--(int){
            size_t current_key_bucket_idx = this->owner->search_buckets(key);
            std::optional<size_t> possibly_removed_key_idx = this->owner->ordered_buckets.at(current_key_bucket_idx).search_contents(key);
            if (not possibly_removed_key_idx.has_value()){
                this->out_of_bounds = true;
                return;
            }
            size_t current_key_idx = possibly_removed_key_idx.value();
            if (current_key_idx == 0){
                do {
                    current_key_bucket_idx--;
                    if (current_key_bucket_idx == SIZE_MAX){
                        out_of_bounds = true;
                        break;
                    }
                } while (this->owner->ordered_buckets.at(current_key_bucket_idx).contents.size() == 0);
                if (not out_of_bounds) {
                    current_key_idx = this->owner->ordered_buckets.at(current_key_bucket_idx).contents.size() - 1;
                }
            } else {
                current_key_idx--;
            }
            if (not out_of_bounds) {
                key = this->owner->ordered_buckets.at(current_key_bucket_idx).contents.at(current_key_idx).first;
            }
        }
        bool is_in_bounds() const {
            return not this->out_of_bounds;
        }
        
        std::pair<key_t,value_t>& operator*() {
            size_t key_bucket_idx = this->owner->search_buckets(key);
            std::optional<size_t> possibly_removed_key_idx = this->owner->ordered_buckets.at(key_bucket_idx).search_contents(key);
            if (not possibly_removed_key_idx.has_value()) {
                throw std::out_of_range("Attempted to dereference invalid iterator");
            } else {
                return owner->ordered_buckets.at(key_bucket_idx).contents.at(possibly_removed_key_idx.value());
            }
        }


    };

    StupidCompTable() {
        this->ordered_buckets.push_back(Bucket{});
    }
    
    StupidCompTable<key_t,value_t>::iterator front(){
        if (this->item_count == 0) {
            return StupidCompTable<key_t,value_t>::iterator{this, true, {}};
        }
        size_t first_item_bucket = 0;
        while (this->ordered_buckets.at(first_item_bucket).contents.size() == 0) {
            first_item_bucket++;
        }
        return StupidCompTable<key_t,value_t>::iterator{this, false, this->ordered_buckets.at(first_item_bucket).contents.at(0).first};
    }

    StupidCompTable<key_t,value_t>::iterator back(){
        if (this->item_count == 0) {
            return StupidCompTable<key_t,value_t>::iterator{this, true, {}};
        }
        size_t last_item_bucket = this->ordered_buckets.size() - 1;
        while (this->ordered_buckets.at(last_item_bucket).contents.size() == 0) {
            last_item_bucket--;
        }
        return StupidCompTable<key_t,value_t>::iterator{this, false, this->ordered_buckets.at(last_item_bucket).contents.back().first};
    }

    StupidCompTable<key_t,value_t>::const_iterator cfront() const {
        if (this->item_count == 0) {
            return StupidCompTable<key_t,value_t>::const_iterator{this, true, {}};
        }
        size_t first_item_bucket = 0;
        while (this->ordered_buckets.at(first_item_bucket).contents.size() == 0) {
            first_item_bucket++;
        }
        return StupidCompTable<key_t,value_t>::const_iterator{this, false, this->ordered_buckets.at(first_item_bucket).contents.at(0).first};
    }

    StupidCompTable<key_t,value_t>::const_iterator cback() const {
        if (this->item_count == 0) {
            return StupidCompTable<key_t,value_t>::const_iterator{this, true, {}};
        }
        size_t last_item_bucket = this->ordered_buckets.size() - 1;
        while (this->ordered_buckets.at(last_item_bucket).contents.size() == 0) {
            last_item_bucket--;
        }
        return StupidCompTable<key_t,value_t>::const_iterator{this, false, this->ordered_buckets.at(last_item_bucket).contents.back().first};
    }

    StupidCompTable<key_t,value_t>::iterator find(key_t lookup_key) {
        Bucket& target_bucket = this->ordered_buckets[this->search_buckets(lookup_key)];
        std::optional<size_t> element_pos = target_bucket.search_contents(lookup_key);
        if (element_pos.has_value()){
            return StupidCompTable<key_t,value_t>::iterator{this, false, target_bucket.contents.at(element_pos.value()).first};
        } else {
            return StupidCompTable<key_t,value_t>::iterator{this, true, {}};
        }
    }

    StupidCompTable<key_t,value_t>::const_iterator find(key_t lookup_key) const {
        const Bucket& target_bucket = this->ordered_buckets[this->search_buckets(lookup_key)];
        std::optional<size_t> element_pos = target_bucket.search_contents(lookup_key);
        if (element_pos.has_value()){
            return StupidCompTable<key_t,value_t>::const_iterator{this, false, target_bucket.contents.at(element_pos.value()).first};
        } else {
            return StupidCompTable<key_t,value_t>::const_iterator{this, true, {}};
        }
    }

    //massive brain engineering here
    size_t size() const {
        return this->item_count;
    }

    //repeat above
    bool empty() const {
        return this->item_count == 0;
    }

    void clear(){
        this->item_count = 0;
        this->ordered_buckets.clear();
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
        Bucket* target_bucket = &this->ordered_buckets[this->search_buckets(lookup_key)];
        std::optional<size_t> element_pos = target_bucket->search_contents(lookup_key);
        if (not element_pos.has_value()) {
            element_pos = target_bucket->insert({ lookup_key,value_t{} }, this->item_count);
        }
        if (this->item_count >= this->ordered_buckets.size() * EXPANSION_THRESHOLD){
            this->reallocate();
            target_bucket = &(this->ordered_buckets.at(this->search_buckets(lookup_key)));
            element_pos = target_bucket->search_contents(lookup_key);
        }
        return target_bucket->contents.at(element_pos.value()).second;
    }

    bool operator==(const StupidCompTable<key_t,value_t>& other) const {
        if (this->item_count != other.item_count){
            return false;
        }
        for (auto own_bucket : this->ordered_buckets){
            for (auto bucket_item : own_bucket.contents){
                auto other_search_result = other.find(bucket_item.first);
                if (not other_search_result.is_in_bounds()){
                    return false;
                } else if (*other_search_result != bucket_item){
                    return false;
                }
            }
        }
        return true;
    }

    bool operator<(const StupidCompTable<key_t,value_t>& other) const {
        //key comparison
        auto own_bucket = this->ordered_buckets.begin();
        auto other_bucket = other.ordered_buckets.begin();
        auto own_key = own_bucket->contents.begin();
        auto other_key = other_bucket->contents.begin();
        for (; not (own_bucket == this->ordered_buckets.end() and other_bucket == other.ordered_buckets.end());){

            if (own_bucket == this->ordered_buckets.end() and other_bucket != other.ordered_buckets.end()){
                return true;
            } else if (other_bucket == other.ordered_buckets.end() and own_bucket != this->ordered_buckets.end()){
                return false;
            }

            if (own_key->first < other_key->first){
                return true;
            } else if (own_key->first > other_key->first) {
                return false;
            }

            own_key++;
            other_key++;
            if (own_key == own_bucket->contents.end()){
                own_bucket++;
                if (own_bucket != this->ordered_buckets.end()){
                    own_key = own_bucket->contents.begin();
                }
            }
            if (other_key == other_bucket->contents.end()){
                other_bucket++;
                if (other_bucket != other.ordered_buckets.end()){
                    other_key = own_bucket->contents.begin();
                }
            }
        }
        //if all keys are identical, value comparison
        own_bucket = this->ordered_buckets.begin();
        other_bucket = other.ordered_buckets.begin();
        auto own_value = own_bucket->contents.begin();
        auto other_value = other_bucket->contents.begin();
        for (; not (own_bucket == this->ordered_buckets.end() and other_bucket == other.ordered_buckets.end());){
            if (own_bucket == this->ordered_buckets.end() and other_bucket != other.ordered_buckets.end()){
                return true;
            } else if (other_bucket == other.ordered_buckets.end() and own_bucket != this->ordered_buckets.end()){
                return false;
            }

            if (own_value->second < other_value->second){
                return true;
            } else if (own_value->second > other_value->second) {
                return false;
            }

            own_value++;
            other_value++;
            if (own_value == own_bucket->contents.end()){
                own_bucket++;
                if (own_bucket != this->ordered_buckets.end()){
                    own_value = own_bucket->contents.begin();
                }
            }
            if (other_value == other_bucket->contents.end()){
                other_bucket++;
                if (other_bucket != other.ordered_buckets.end()){
                    other_value = own_bucket->contents.begin();
                }
            }
        }
        //whole thing is identical then
        return false;
    }

    bool operator>(const StupidCompTable<key_t,value_t>& other) const {
        return other < *this;
    }

    bool operator>=(const StupidCompTable<key_t,value_t>& other) const {
        return not (*this > other);
    }

    bool operator<=(const StupidCompTable<key_t,value_t>& other) const {
        return not (*this < other);
    }
};


