#pragma once


#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>

#include <SDL3/SDL_render.h>


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <cmath>
#include <algorithm>
#include <functional>
#include <cstdint>

template<typename T>
using Vector = std::vector<T>;

template<typename t>
using List = std::list<t>;

template<typename T>
using Ref = std::shared_ptr<T>;

template <typename T>
using VectorRef = std::vector<Ref<T>>;

template <typename T>
using ListRef = std::list<Ref<T>>;

template<typename T,typename R>
using UMap = std::unordered_map<T, R>;

template<typename T, typename R>
using Map = std::map<T, R>;

template<typename T, typename R>
using MapRef = std::map<T, Ref<R>>;

template<typename T, typename R>
using UMapRef = std::unordered_map<T, Ref<R>>;

using String = std::string;

template<typename T, std::size_t C>
using Array = std::array<T, C>;

template<typename T, std::size_t C>
using ArrayRef = std::array<Ref<T>,C> ;


template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... arghs) {
	return std::make_shared<T>(std::forward<Args>(arghs)...);
}

template<typename Base, typename Derived>
auto StaticCast(Derived& d) {
	return std::static_pointer_cast<Base>(d);
}

template<typename Base, typename Derived>
auto DynamicCast(Derived& d) {
	return std::dynamic_pointer_cast<Base>(d);
}

inline auto farmMin(auto a, auto b) {
	return (a < b) ? a : b;
}

inline auto farmMax(auto a, auto b) {
	return (a > b) ? a : b;
}


#include "Vector2.h"
#include "BinaryIO.h"
#include "Random.h"
#include "Window.h"
#include "Renderer.h"
#include "Primitives.h"
#include "Animation.h"
#include "Helper.h"
#include "AnimSerialization.h"
#include "RenderGlyphs.h"
#include "UILayer.h"
#include "Game.h"
#include "Main.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
