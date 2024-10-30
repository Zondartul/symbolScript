#include <cassert>
/// vector span. acts like a view into a vector and lets you compare
/// parts of vectors without copying.
/// todo: make a const-qualified counterpart cSpan.

template<typename T> class Span_const;

template<typename T> class Span{
    public:
    using V = typename std::vector<T>;
    using iter = typename V::iterator;

    V &vec;
    int offs = 0;
    int len = 0;
    
    Span(std::vector<T>& vec, int offs=0, int len=-1);
    iter begin();
    iter end();
    template<typename K> bool operator==(Span<K> &other);
    template<typename K> bool operator==(const Span_const<K> &other) const;
    void erase();
};

template<typename T> class Span_const{
    public:
    using V = typename std::vector<T>;
    using citer = typename V::const_iterator;
    
    const V &vec;
    int offs = 0;
    int len = 0;

    Span_const(const std::vector<T>& vec, int offs=0, int len=-1);
    Span_const(const Span<T> &other);
    citer begin() const;
    citer end() const;
    template<typename K> bool operator==(const Span<K> &other) const;
    template<typename K> bool operator==(Span_const<K> &other) const;
};

///--------- implementation --------------

///------ Span (non-const) --------------
template<typename T>
Span<T>::Span(std::vector<T>& vec, int new_offs, int new_len): vec(vec),offs(new_offs),len(new_len){
    assert(new_offs >= 0);
    if(new_len == -1){len = vec.size();}
}

template<typename T>
typename Span<T>::iter 
Span<T>::begin(){
    return vec.begin() + offs;
}

template<typename T>
typename Span<T>::iter 
Span<T>::end(){
    //if(len == -1){return vec.end();}
    //else{
    /// bounds checking!
    unsigned int offs_safe;
    if((offs + len) > vec.size()){
        if(offs > vec.size()){
            offs_safe = (unsigned int)offs; /// end is still beyond the boundary but iteration stops
        }else{
            offs_safe = vec.size(); /// end is at the boundary
        }
    }else{
        offs_safe = (unsigned int)(offs + len); /// end is before or at the boundary
    }    

    return vec.begin() + offs_safe;
    //}
}

template<typename T>
template<typename K> 
bool 
Span<T>::operator==(Span<K> &other){
    using iterT = iter;
    using iterK = typename Span<K>::iter;

    if(((void*)&vec == (void*)&other.vec) && (offs == other.offs) && (len == other.len)){
        return true;
    }else if(len == other.len){
        iterT I = begin(); 
        iterT Ie = end();
        iterK J = other.begin(); 
        iterK Je = other.end();

        while((I != Ie) && (J != Je)){
            if(!(*I == *J)){return false;}
            I++; J++;
        }
        return true;
    }else{
        return false;
    }
}

template<typename T> 
template<typename K> 
bool 
Span<T>::operator==(const Span_const<K> &other) const{
    return Span_const<T>(*this) == other;
}

template<typename T> 
void 
Span<T>::erase(){vec.erase(begin(), end());}

///--------- Span_const -------------------

template<typename T>
Span_const<T>::Span_const(const std::vector<T>& vec, int new_offs, int new_len):
    vec(vec), offs(new_offs), len(new_len) {
    assert(new_offs >= 0);
    if(new_len == -1){len = vec.size();}
    }

template<typename T>
Span_const<T>::Span_const(const Span<T> &other):
    vec(other.vec), offs(other.offs), len(other.len) {}

template<typename T>
typename Span_const<T>::citer 
Span_const<T>::begin() const{
    return vec.cbegin() + offs;
}

template<typename T>
typename Span_const<T>::citer 
Span_const<T>::end() const{
    //if(len == -1){return vec.cend();}
    //else{
    /// bounds checking!
    unsigned int offs_safe;
    if((offs + len) > vec.size()){
        if(offs > vec.size()){
            offs_safe = (unsigned int)offs; /// end is still beyond the boundary but iteration stops
        }else{
            offs_safe = vec.size(); /// end is at the boundary
        }
    }else{
        offs_safe = (unsigned int)(offs + len); /// end is before or at the boundary
    }    

    return vec.cbegin() + offs_safe;
    //}
}

template<typename T>
template<typename K> 
bool 
Span_const<T>::operator==(const Span<K> &other) const{
    return *this == Span_const<K>(other);
}

template<typename T>
template<typename K> 
bool 
Span_const<T>::operator==(Span_const<K> &other) const{
    using iterT = citer;
    using iterK = typename Span<K>::citer;

    if(((void*)&vec == (void*)&other.vec) && (offs == other.offs) && (len == other.len)){
        return true;
    }else if(len == other.len){
        iterT I = begin(); 
        iterT Ie = end();
        iterK J = other.begin(); 
        iterK Je = other.end();

        while((I != Ie) && (J != Je)){
            if(!(*I == *J)){return false;}
            I++; J++;
        }
        return true;
    }else{
        return false;
    }
}