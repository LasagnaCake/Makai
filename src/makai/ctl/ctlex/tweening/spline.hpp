#ifndef CTL_EX_TWEENING_SPLINES_H
#define CTL_EX_TWEENING_SPLINES_H

#include "../math/vector.hpp"
#include "../../ctl/math/core.hpp"
#include "../../ctl/container/list.hpp"
#include "../../ctl/container/arguments.hpp"
#include "../../ctl/exnamespace.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Spline facilities.
namespace Spline {
	/// @brief Spline interpolator interface.
	/// @tparam T Value type.
	template<CTL::Type::Math::Operatable T>
	struct Splinoid {
		typedef T DataType;

		constexpr virtual T interpolate(float const by) const = 0;
	};
}

/// @brief Spline-specific type constraints.
namespace Type::Ex::Spline {
	template<class T>
	concept Spline = CTL::Type::Subclass<T, CTL::Ex::Spline::Splinoid<typename T::DataType>>;
}

/// @brief Spline facilities.
namespace Spline {
	/// @brief Linear spline interpolator.
	/// @tparam T Value type.
	template<class T>
	class Linear: public Splinoid<T> {
	public:
		/// @brief Points to interpolate between.
		List<T> points;

		/// @brief Empty constructor.
		constexpr Linear() {}

		/// @brief Constructs the spline with a series of points.
		/// @param ps Points to use.
		constexpr Linear(List<T> const& ps): points(ps) {}

		/// @brief Constructs the spline with a series of points.
		/// @param ps Points to use.
		constexpr Linear(Arguments<T> const& ps) {
			points.resize(ps.size());
			for (T& p: ps)
				points.pushBack(p);
		}

		/// @brief Constructs the spline with an array of points.
		/// @tparam N Array size.
		/// @param ps Points to use.
		template<usize N>
		constexpr Linear(Decay::AsType<T const[N]> const& ps) {
			points.resize(N);
			for (T& p: ps)
				points.pushBack(p);
		}

		/// @brief Interpolates between the given points. 
		/// @param by Interpolation factor.
		/// @return Interpolated value.
		constexpr T interpolate(float by) const final {
			by = ::CTL::Math::clamp<float>(by, 0, 1);
			if (by == 1.0) return points.end();
			usize curp = ::CTL::Math::floor(by * points.size());
			return Math::lerp(points[curp], points[curp+1], by);
		}
	};

	/// @brief Bezier splines.
	namespace Bezier {
		/// @brief Bezier spline section.
		/// @tparam T Value type.
		/// @tparam N Number of elements.
		template<CTL::Type::Math::Operatable T, usize N>
		struct Section {
			T points[N];
		};

		/// @brief List of bezier sections.
		/// @tparam T Section value type.
		/// @tparam N Section size.
		template<CTL::Type::Math::Operatable T, usize N>
		using SectionList = List<Section<T, N>>;

		/// @brief Bezier spline interpolator.
		/// @tparam T Value type.
		/// @tparam N Section size (spline type).
		template<CTL::Type::Math::Operatable T, usize N>
		class Spline: public Splinoid<T> {
		public:
			/// @brief Empty constructor.
			constexpr Spline() {}

			/// @brief Constructs the spline from a series of sections.
			/// @param secs Sections to use.
			constexpr Spline(SectionList<T, N> const& secs) {
				sections = secs;
			}

			/// @brief Constructs the spline from a series of sections.
			/// @param secs Sections to use.
			constexpr Spline(Arguments<Section<T, N>> const& secs) {
				sections.resize(secs.size());
				for (Section<T, N>& s: secs)
					sections.pushBack(s);
			}

			/// @brief Constructs the spline from an array of point groups.
			/// @tparam P Array size.
			/// @param points Points to use.
			template <usize P>
			constexpr Spline(Decay::AsType<T const[P][N]> const& points) {
				sections.resize(P);
				for (usize i = 0; i < P; ++i) {
					Section<T, N> sec;
					for (usize j = 0; j < N; ++j) 
						sec.points[j] = points[i][j];
					sections.pushBack(sec);
				}
			}

			/// @brief Constructs the spline from an array of points.
			/// @tparam P Array size.
			/// @param points Points to use.
			/// @note Requires point count to be a multiple of the section size.
			template <usize P>
			constexpr Spline(Decay::AsType<T const[P]> const& points) {
				static_assert(P % N == 0, "Point count is not a multiple of section size!");
				sections.resize(P/N);
				for (usize i = 0; i < P; i += N) {
					Section<T, N> sec;
					for (usize j = 0; j < N; ++j) 
						sec.points[j] = points[i+j];
					sections.pushBack(sec);
				}
			}

			/// @brief Sections to interpolate between.
			SectionList<T, N> sections;

			/// @brief Interpolates between the given points. 
			/// @param by Interpolation factor.
			/// @return Interpolated value.
			constexpr T interpolate(float by) const final {
				by = ::CTL::Math::clamp<float>(by, 0, 1);
				if (by == 1.0) return sections.end()[0];
				usize sec = ::CTL::Math::floor(by * sections.size());
				return lerpSection(sections[sec], sections[sec+1].points[0], by);
			}

		private:
			template<usize S>
			constexpr T lerpSection(Section<T, S> const& sec, T const& end, float const by) {
				if constexpr(S < 3)
					return ::CTL::Math::lerp(sec.points[0], end, T(by));
				else {
					Section<T, S-1> res;
					T newEnd = Math::lerp(sec.points[S-1], end, T(by));
					for (usize i = 0; i < S-2; ++i)
						res.points[i] = ::CTL::Math::lerp(sec.points[S], sec.points[S+1], T(by));
					return lerpSection(res, newEnd, by);
				}
			}
		};

		/// @brief `Spline` analog for quadratic bezier splines.
		/// @tparam T Value type.
		template<CTL::Type::Math::Operatable T> using Quadratic	= Spline<T, 2>;
		/// @brief `Spline` analog for cubic bezier splines.
		/// @tparam T Value type.
		template<CTL::Type::Math::Operatable T> using Cubic		= Spline<T, 3>;
		/// @brief `Spline` analog for quartic bezier splines.
		/// @tparam T Value type.
		template<CTL::Type::Math::Operatable T> using Quartic	= Spline<T, 4>;
		/// @brief `Spline` analog for quintic bezier splines.
		/// @tparam T Value type.
		template<CTL::Type::Math::Operatable T> using Quintic	= Spline<T, 5>;
	}

	/// @brief Hermite splines.
	namespace Hermite {
		/// @brief Hermite spline section.
		/// @tparam T Value type.
		template<CTL::Type::Math::Operatable T>
		struct Section {
			T position;
			T velocity;
		};

		/// @brief List of hermite sections.
		/// @tparam T Value type.
		template<typename T>
		using SectionList = List<Section<T>>;

		/// @brief Hermite spline interpolator.
		/// @tparam T Value type.
		template<CTL::Type::Math::Operatable T>
		class Spline: public Splinoid<T> {
		public:
			/// @brief Empty constructor.
			constexpr Spline() {}

			/// @brief Constructs the spline from a series of sections.
			/// @param secs Sections to use.
			constexpr Spline(SectionList<T> const& secs) {
				sections = secs;
			}

			/// @brief Constructs the spline from a series of sections.
			/// @param secs Sections to use.
			constexpr Spline(Arguments<Section<T>> const& secs) {
				sections.resize(secs.size());
				for (Section<T>& s: secs)
					sections.pushBack(s);
			}

			/// @brief Constructs the spline from an array of point groups.
			/// @tparam P Array size.
			/// @param points Points to use.
			template <usize P>
			constexpr Spline(Decay::AsType<T const[P][2]> const& points) {
				sections.resize(P);
				for (usize i = 0; i < P; ++i) {
					sections.pushBack(
						Section<T> {
							points[i][0],
							points[i][1]
						}
					);
				}
			}

			/// @brief Constructs the spline from an array of points.
			/// @tparam P Array size.
			/// @param points Points to use.
			/// @note Requires point count to be a multiple of 2.
			template<usize P>
			constexpr Spline(Decay::AsType<T const[P]> const& points) {
				sections.resize(P/2);
				static_assert(P % 2 == 0, "Point count is not a multiple of 2!");
				for (usize i = 0; i < P; i += 2) {
					sections.pushBack(
						Section<T> {
							points[i],
							points[i+1]
						}
					);
				}
			}

			/// @brief Sections to interpolate between.
			SectionList<T> sections;

			/// @brief Interpolates between the given points. 
			/// @param by Interpolation factor.
			/// @return Interpolated value.
			constexpr T interpolate(float by) const final {
				by = ::CTL::Math::clamp<float>(by, 0, 1);
				if (by == 1.0) return sections.end().position;
				usize sec = ::CTL::Math::floor(by * sections.size());
				return lerpSection(sections[sec], sections[sec+1], by);
			}

		private:
			constexpr T lerpSection(Section<T> const& sec, Section<T> const& next, float const by) {
				T const pos[2] = {
					sec.position + sec.velocity,
					next.position - next.velocity
				};
				T const p1[3] = {
					::CTL::Math::lerp(sec.position, pos[0], T(by)),
					::CTL::Math::lerp(pos[0], pos[1], T(by)),
					::CTL::Math::lerp(pos[1], next.position, T(by))
				};
				T const p2[2] = {
					::CTL::Math::lerp(p1[0], p1[1], T(by)),
					::CTL::Math::lerp(p1[1], p1[2], T(by))
				};
				return ::CTL::Math::lerp(p2[0], p2[1], T(by));
			}
		};
	}
}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_TWEENING_SPLINES_H
