#ifndef CTL_EX_TWEENING_TWEEN_H
#define CTL_EX_TWEENING_TWEEN_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/container/arguments.hpp"
#include "../../ctl/container/list.hpp"
#include "../../ctl/container/functor.hpp"
#include "../math/ease.hpp"
#include "../event/timer.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Tween-specific type constraints.
namespace Type::Ex::Tween {
	/// @brief Type must be operatable, and must be constructible from a single float.
	template <typename T>
	concept Tweenable = CTL::Type::Math::Operatable<T> && CTL::Type::Constructible<T, float>;
}

template <Type::Ex::Tween::Tweenable T = float>
class Tween;

/// @brief Tween-specific periodic event interface.
using ITweenPeriodic = IPeriodic<Tween<>, usize>;

/// @brief Stepable tween interface.
struct IStepable {
	/// @brief Returns the current step.
	/// @return Current step.
	virtual usize		getStep()					{return step;	}
	/// @brief Sets the current step. Must be implemented.
	/// @param . Step to set to.
	/// @return Reference to self.
	virtual IStepable&	setStep(usize const)		= 0;
	/// @brief Sets the total step count. Must be implemented.
	/// @param . Step count to set to.
	/// @return Reference to self.
	virtual IStepable&	setStepCount(usize const)	= 0;

protected:
	/// @brief Current step.
	usize step = 0;
};

/// @brief Tween-specific playable event interface.
struct ITweenPlayable: IPlayable {
	/// @brief Ends the tween's execution, and sets its value to the end value.
	/// @return Reference to self.
	virtual ITweenPlayable& conclude()	= 0;
	/// @brief Ends the tween's execution, while keeping its current value.
	/// @return Reference to self.
	virtual ITweenPlayable& halt()		= 0;

	/// @brief Signal to be fired upon completion.
	::CTL::Signal<> onCompleted;

private:
	virtual ITweenPlayable& stop()		{return halt();}
};

/// @brief Periodic value interpolator.
/// @tparam T Value type.
template <Type::Ex::Tween::Tweenable T>
class Tween:
	public ITweenPeriodic,
	public ITweenPlayable,
	public IValue<T>,
	public IStepable {
public:
	/// @brief Value type.
	typedef T DataType;

	using ITweenPeriodic::ITweenPeriodic;

	/// @brief Easing function to use.
	Math::Ease::Mode easeMode = Math::Ease::linear;
	/// @brief Start value.
	T from;
	/// @brief End value.
	T to;

	/// @brief Constructs the tween with a series of starting values.
	/// @param from Start value.
	/// @param to End value.
	/// @param step Step count.
	/// @param easeMode Ease function. By default, it is `Math::Ease::linear`.
	/// @param manual Whether the tween is processed manually. By default, it is `false`.
	Tween(
		T const& from,
		T const& to,
		usize const step,
		Math::Ease::Mode const& easeMode = Math::Ease::linear,
		bool const manual = false
	)
	: ITweenPeriodic(manual) {
		setInterpolation(from, to, step, easeMode);
	}

	/// @brief Move constructor.
	/// @param other `Tween` to move.
	Tween(Tween&& other)
	: Tween(
		other.from,
		other.to,
		other.stop,
		other.easeMode,
		other.isManual()
	) {
		paused = other.paused;
		onCompleted = other.onCompleted;
		isFinished = other.isFinished;
		factor = other.factor;
		current = other.current;
		step = other.step;
		other.setManual();
	}

	/// @brief Copy constructor (deleted).
	Tween(Tween const& other) = delete;

	/// @brief Destructor.
	virtual ~Tween() {}

	/// @brief Processes a step.
	/// @param delta Delta between steps.
	void onUpdate(usize delta) override final {
		// If paused, return
		if (paused) return;
		// If not finished...
		if (!isFinished) {
			// Check if finished, then increment step
			isFinished = step >= stop;
			step += delta;
			// If begin != end, calculate step
			if (from != to) {
				factor = easeMode(float(step)/float(stop));
				current = Math::lerp(from, to, T(factor));
			}
			// Else, set target to end
			else current = to;
			// Clamp step to prevent overflow
			step = (step > stop ? stop : step);
			// If done, fire signal
			if (isFinished)
				onCompleted();
		}
	}

	/// @brief Sets the interpolation.
	/// @param from Start value.
	/// @param to End value.
	/// @param step Step count.
	/// @param easeMode Ease function.
	/// @return Reference to self.
	Tween<T>& setInterpolation(T const& from, T const& to, usize const step, Math::Ease::Mode const& easeMode) {
		isFinished = false;
		this->step = 0;
		this->from = from;
		this->to = to;
		stop = step;
		value = from;
		this->easeMode = easeMode;
		factor = 0.0f;
		return *this;
	}

	/// @brief
	///		Sets the interpolation to a new value,
	///		while maintaining the easing function.
	///		Current value is used as start value.
	/// @param to New end value.
	/// @return Reference to self.
	Tween<T>& reinterpolateTo(T const& to) {
		paused = false;
		setInterpolation(value, to, step, easeMode);
		return *this;
	}

	/// @brief
	///		Sets the interpolation to a new value,
	///		while maintaining the easing function.
	/// @param from New start value.
	/// @param to New end value.
	/// @return Reference to self.
	Tween<T>& reinterpolate(T const& from, T const& to) {
		paused = false;
		setInterpolation(from, to, step, easeMode);
		return *this;
	}

	/// @brief
	///		Sets the interpolation to a new value,
	///		while maintaining the easing function.
	///		Current value is used as start value.
	/// @param to New end value.
	/// @param step New step count.
	/// @return Reference to self.
	Tween<T>& reinterpolateTo(T const& to, usize const step) {
		paused = false;
		setInterpolation(value, to, step, easeMode);
		return *this;
	}

	/// @brief
	///		Sets the interpolation to a new value,
	///		while maintaining the easing function.
	/// @param from New start value.
	/// @param to New end value.
	/// @param step New step count.
	/// @return Reference to self.
	Tween<T>& reinterpolate(T const& from, T const& to, usize const step) {
		paused = false;
		setInterpolation(from, to, step, easeMode);
		return *this;
	}

	/// @brief Sets the current tween step.
	/// @param step Step to set to.
	/// @return Reference to self.
	Tween<T>& setStep(usize const step) override final {
		this->step = step > stop ? stop : step;
		onUpdate();
		return *this;
	}

	/// @brief Sets the current tween step count.
	/// @param stop Step count to set to.
	/// @return Reference to self.
	Tween<T>& setStepCount(usize const stop) override final {
		this->stop = stop;
		return *this;
	}

	/// @brief Ends the tween's execution, and sets its value to the end value.
	/// @return Reference to self.
	Tween<T>& conclude() override final {
		step = stop;
		factor = 1.0f;
		isFinished = true;
		return *this;
	}

	/// @brief Resumes the tween.
	/// @return Reference to self.
	Tween<T>& play() override final {
		factor = 1.0f;
		paused = false;
		isFinished = false;
		return *this;
	}

	/// @brief Pauses the tween.
	/// @return Reference to self.
	Tween<T>& pause() override final {
		paused = true;
		return *this;
	}

	/// @brief Ends the tween's execution, while keeping its current value.
	/// @return Reference to self.
	Tween<T>& halt() override final {
		factor = 1.0f;
		isFinished = true;
		return *this;
	}

	/// @brief Returns the current value.
	/// @return Current value.
	T value() const override final {
		return current;
	}

private:
	using
		IStepable::step,
		ITweenPlayable::isFinished
	;

	/// @brief Current value.
	T current;
	/// @brief Interpolation factor used for calculating the current value.
	float factor = 1.0f;
	/// @brief Amount of steps to be processed.
	usize stop;
};

/// @brief Tween interpolation stage.
/// @tparam T Value type.
template<Type::Ex::Tween::Tweenable T = float>
struct StageData {
	typedef T DataType;
	/// @brief Start value.
	DataType from;
	/// @brief End value.
	DataType to;
	/// @brief Step count.
	usize step;
	/// @brief Ease function to use. By default, it is `Math::Ease::linear`.
	Math::Ease::Mode ease	= Math::Ease::linear;
	/// @brief Action to do when this stage is finished. By default, does nothing.
	::CTL::Signal<> onCompleted = []{};
};

/// @brief Stageable tween interface.
struct IStageable: IStepable {
	/// @brief Sets the current stage. Must be implemented.
	/// @param . Stage to set to.
	/// @return Reference to self.
	virtual IStageable&	setStage(usize const)		= 0;
	/// @brief Returns the current stage.
	/// @return Current stage.
	virtual usize		getStage()					{return stage;	}

protected:
	/// @brief Current stage.
	usize stage = 0;

private:
	virtual IStageable&	setStepCount(usize const)	{return *this;	}
};

/// @brief Periodic value interpolator, with multiple stages.
/// @tparam T Value type.
template<Type::Ex::Tween::Tweenable T = float>
class TweenChain:
	public ITweenPeriodic,
	public ITweenPlayable,
	public IValue<T>,
	public IStageable {
public:
	/// @brief Value type.
	typedef T DataType;

	/// @brief Tween stage type.
	typedef StageData<DataType>	Stage;
	/// @brief Tween stage list type.
	typedef List<Stage>			StageList;
	/// @brief Tween stage argument list type.
	typedef Arguments<Stage>	StageArguments;

	using ITweenPeriodic::ITweenPeriodic;

	/// @brief Constructs the tween with a set of stages.
	/// @param stages Stages to interpolate between.
	/// @param manual Whether the tween is processed manually. By default, it is `false`.
	TweenChain(StageList const& stages, bool const manual = false)
	: ITweenPeriodic(manual) {
		setInterpolation(stages);
	}

	/// @brief Constructs the tween with a set of stages.
	/// @param stages Stages to interpolate between.
	/// @param manual Whether the tween is processed manually. By default, it is `false`.
	TweenChain(StageArguments const& stages, bool const manual = false)
	: ITweenPeriodic(manual) {
		setInterpolation(stages);
	}

	/// @brief Move constructor.
	/// @param other `TweenChain` to move.
	TweenChain(TweenChain&& other): TweenChain(other.stages, other.isManual()) {
		paused = other.paused;
		isFinished = other.isFinished;
		factor = other.factor;
		current = other.current;
		currentValue = other.currentValue;
		stage = other.stage;
		other.setManual();
	}

	/// @brief Copy constructor (deleted).
	TweenChain(TweenChain const& other) = delete;

	/// @brief Destructor.
	virtual ~TweenChain() {}
	
	/// @brief Processes a step.
	/// @param delta Delta between steps.
	void onUpdate(usize delta) override final {
		// If paused, return
		if (paused) return;
		// If not finished...
		if (!isFinished) {
			// If current stage is done...
			if (step >= current.step) {
				// Fire stage's onCompleted signal;
				current.onCompleted();
				// Check if truly finished, else continue on to next stage
				if (!(isFinished = stage >= stages.size())) {
					current	= stages[stage++];
					step	= 0;
				}
			}
			// Increment step counter
			step += delta;
			// If begin != end, calculate step
			if (current.from != current.to) {
				factor = current.ease(float(step)/float(current.step));
				currentValue = Math::lerp(current.from, current.to, T(factor));
			}
			// Else, set target to end
			currentValue = current.to;
			// Clamp step to prevent overflow
			step = (step > current.step ? current.step : step);
			// If done, fire signal
			if (isFinished)
				onCompleted();
		}
	}

	/// @brief Sets the interpolation.
	/// @param stages Stages to interpolate between.
	/// @return Reference to self.
	TweenChain& setInterpolation(StageList const& stages) {
		this->stages = stages;
		return start();
	}

	/// @brief Sets the interpolation.
	/// @param stages Stages to interpolate between.
	/// @return Reference to self.
	TweenChain& setInterpolation(StageArguments const& stages) {
		return setInterpolation(StageList(stages));
	}

	/// @brief Sets the current tween step.
	/// @param step Step to set to.
	/// @return Reference to self.
	TweenChain& setStep(usize const step) override final {
		this->step = (step-1) > current.step ? current.step : (step-1);
		onUpdate();
		return *this;
	}

	/// @brief Sets the current tween stage.
	/// @param step Stage to set to.
	/// @return Reference to self.
	TweenChain& setStage(usize const stage) override final {
		this->stage = stage >= stages.size() ? stages.size() : stage;
		onUpdate();
		return *this;
	}

	/// @brief Gets the current tween stage.
	/// @return Current stage.
	usize getStage() override final {
		return stage;
	}

	/// @brief Gets the current tween step in the stage.
	/// @return Current step in stage.
	usize getStep() override final {
		return step;
	}

	/// @brief Resumes the tween.
	/// @return Reference to self.
	TweenChain& play() override final {
		factor = 1.0f;
		paused = false;
		isFinished = false;
		return *this;
	}

	/// @brief Pauses the tween.
	/// @return Reference to self.
	TweenChain& pause() override final {
		paused = true;
		return *this;
	}

	/// @brief Starts the tween from the beginning.
	/// @return Reference to self.
	TweenChain& start() override final {
		step = 0;
		stage = 0;
		current = stages[stage++];
		return play();
	}

	/// @brief Ends the tween's execution, while keeping its current value.
	/// @return Reference to self.
	TweenChain& halt() override final {
		factor = 1.0f;
		isFinished = true;
		return *this;
	}

	/// @brief Ends the tween's execution, and sets it value to the final stage's end value.
	/// @return Reference to self.
	TweenChain& conclude() override final {
		step = stages.front().step;
		return halt();
	}

	/// @brief Returns the current value.
	/// @return Current value.
	T value() const override final {
		return currentValue;
	}

	/// @brief Stages to interpolate between.
	StageList stages;

private:
	using
		IStageable::step,
		IStageable::stage,
		ITweenPlayable::isFinished
	;

	/// @brief Current stage being processed.
	Stage current;

	/// @brief Current value.
	T currentValue;

	/// @brief Interpolation factor used for calculating the current value.
	float factor = 1.0f;
};

CTL_EX_NAMESPACE_END

#endif // CTL_EX_TWEENING_TWEEN_H
