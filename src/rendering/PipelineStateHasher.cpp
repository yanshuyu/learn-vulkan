#include<rendering\PipelineStateHasher.h>
#include<regex>
#include<stdarg.h>

void hash_combine(size_t &seed, size_t hash)
{
    hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash;
}

 


size_t hash_fundamentals(const char* types, ...)
{
    std::string fmt_str = types;
    std::vector<std::string> type_strs{};
    std::regex reg("%[a-z]+");
    auto beg = std::sregex_iterator(fmt_str.begin(), fmt_str.end(), reg);
    auto end = std::sregex_iterator();
    for (auto matchItr = beg; matchItr != end; ++matchItr)
    {
        auto match = *matchItr;
        type_strs.push_back(match.str());
    }

    if (type_strs.size() <= 0)
        return 0;

    size_t h{0};
    std::va_list vl{};
    va_start(vl, types);
    for (auto &&t : type_strs)
    {
        if (t == "%d" || t == "%i" ) // signed integer
        {
            int32_t i = va_arg(vl, int32_t);
            hash_combine(h, std::hash<int32_t>{}(i));
        }
        else if (t == "%ud" || t == "%ui") // unsigned integer
        {
            uint32_t ui = va_arg(vl, uint32_t);
            hash_combine(h, std::hash<uint32_t>{}(ui));
        } 
        else if (t == "%ld" || t == "%li") // long integer
        {
            long l = va_arg(vl, long);
            hash_combine(h, std::hash<long>{}(l));
        }
        else if (t == "%ul") // unsigned long integer
        {
            unsigned long ul = va_arg(vl, unsigned long);
            hash_combine(h, std::hash<unsigned long>{}(ul));
        }
        else if (t == "%ll") // long long integer
        {
            long long ll = va_arg(vl, long long);
            hash_combine(h, std::hash<long long>{}(ll));
        }
        else if (t == "%ull") // unsigned long long integer
        {
            unsigned long long ull = va_arg(vl, unsigned long long);
            hash_combine(h, std::hash<unsigned long long>{}(ull));
        }
        else if (t == "%f") // float
        {
            float f = va_arg(vl, float);
            hash_combine(h, std::hash<float>{}(f));
        }
        else if (t == "%lf") // double
        {
            double d = va_arg(vl, double);
            hash_combine(h, std::hash<double>{}(d));
        }
        else if (t == "%c") // charater
        {   
            char c = va_arg(vl, char);
            hash_combine(h, std::hash<char>{}(c));
        }
        else if (t == "%s") // string or sequence of charater
        {
            const char* s = va_arg(vl, const char*);
            hash_combine(h, std::hash<std::string>{}(s));
        }
        else if (t == "%b")
        {
            bool b = va_arg(vl, bool);
            hash_combine(h, std::hash<bool>{}(b));
        }
        else 
        {
            va_end(vl);
            throw std::invalid_argument("unsupported type!");
        }
    }
    va_end(vl);
    
    return h;
}
