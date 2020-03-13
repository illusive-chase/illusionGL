#pragma once
namespace illusion {
	template<typename ...T> struct fill;
	template<typename L> struct fill<L> {
		void operator()(void* ptr, L&& arg) { *reinterpret_cast<typename std::remove_reference<L>::type *>(ptr) = std::forward<L>(arg); }
	};
	template<typename F, typename ...R> struct fill<F, R...> {
		void operator()(void* ptr, F&& arg, R&&... args) {
			*reinterpret_cast<typename std::remove_reference<F>::type *>(ptr) = std::forward<F>(arg);
			fill<R...>()(reinterpret_cast<typename std::remove_reference<F>::type *>(ptr) + 1, args...);
		}
	};

	template<typename RETURN, typename ...T> struct fill_with_return;
	template<typename RETURN, typename L> struct fill_with_return<RETURN, L> {
		void operator()(RETURN (std::remove_reference<L>::type::* func)() const, void* ptr, L&& arg) { *reinterpret_cast<RETURN*>(ptr) = (arg.*func)(); }
	};
	template<typename RETURN, typename F, typename ...R> struct fill_with_return<RETURN, F, R...> {
		void operator()(RETURN(std::remove_reference<F>::type::* func)() const, void* ptr, F&& arg, R&& ... args) {
			*reinterpret_cast<RETURN*>(ptr) = (arg.*func)();
			fill_with_return<RETURN, R...>()(func, reinterpret_cast<RETURN*>(ptr) + 1, args...);
		}
	};

}