#include <iostream>
#include <type_traits>

template<typename T, int N>
struct property_counter { 
	friend constexpr void adl_flag(property_counter<T, N>); 
};

template<typename T, int N>
struct writer {
	friend constexpr void adl_flag(property_counter<T, N>) { }
	static constexpr int value = N;
};

template<typename T, int N, typename = std::enable_if_t<noexcept(adl_flag(property_counter<T, N>()))>>
int constexpr reader(int, property_counter<T, N>) {
	return N;
}

template<typename T, int N>
int constexpr reader(float, property_counter<T, N>, int R = reader(0, property_counter<T, N - 1>())) {
	return R;
}

template <typename T>
int constexpr reader(float, property_counter<T, 0>) {
	return 0;
}

template<typename T, int C = reader(0, property_counter<T, 32>())>
int constexpr count_property(int R = C) {
	writer<T, C + 1>();
	return R;
}

template <typename C, typename T>
class member
{
	T C::* mem_ptr;
};

#define REGISTER_ENTITY(entity) \
protected: \
	using parent_t = this_t; \
	using this_t = entity; \
	writer<this_t, reader(0, property_counter<parent_t, 32>())> temp; \
public: \
	template <size_t N> \
	static void Property() { parent_t::Property<N>(); } \

#define REGISTER_FIELD(member) \
	template <> \
	static void Property<count_property<this_t>()>() { std::cout << #member << "\n"; }

struct base_struct
{
protected:
	using this_t = base_struct;
public:
	template <size_t>
	static void Property() { std::cout << "default\n"; }

	REGISTER_FIELD(member_0)
};

struct derived_struct : public base_struct
{
	REGISTER_ENTITY(derived_struct)

	REGISTER_FIELD(member_1)
};

struct derived_struct1 : public derived_struct
{
	REGISTER_ENTITY(derived_struct1)
	
	REGISTER_FIELD(member_2)
	REGISTER_FIELD(member_3)
};

void main()
{
	derived_struct::Property<0>();
	derived_struct::Property<1>();
	derived_struct::Property<2>();

	derived_struct1::Property<0>();
	derived_struct1::Property<1>();
	derived_struct1::Property<2>();
	derived_struct1::Property<3>();
	derived_struct1::Property<4>();
}