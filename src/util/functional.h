#include <functional>
#include <tuple>

namespace func {
template <class F, class G>
auto compose(F f, G g) {
    return [f, g](auto x) { return f(g(x)); };
}

template <class F, class G>
auto operator*(F f, G g) {
    return compose(f, g);
}

}  // namespace func
