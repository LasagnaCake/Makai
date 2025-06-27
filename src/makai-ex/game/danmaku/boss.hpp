#ifndef MAKAILIB_EX_GAME_DANMAKU_BOSS_H
#define MAKAILIB_EX_GAME_DANMAKU_BOSS_H

#include "enemy.hpp"
#include "makai/core/app.hpp"

/// @brief Danmaku-specific game extensions.
namespace Makai::Ex::Game::Danmaku {
	/// @brief Boss enemy.
	struct ABoss: AEnemy {
		/// @brief Boss battle act.
		struct AAct: Co::ARoutineTask {
			/// @brief Virtual destructor.
			virtual ~AAct() {}
			/// @brief Boss associated with the act.
			ABoss& boss;
			/// @brief Consructs the act.
			/// @param boss Boss associated with the act.
			AAct(ABoss& boss): boss(boss) {}
			/// @brief Called every update cycle.
			virtual void onUpdate(float, Makai::App&) {ARoutineTask::process();}
			/// @brief Returns the act that follows this one. Must be implemented.
			virtual usize next() const = 0;
		};

		/// @brief Act instance container type.
		using ActInstanceType = Makai::Unique<AAct>;

		/// @brief Constructs the boss.
		/// @param cfg Enemy configuration.
		ABoss(EnemyConfig const& cfg): AEnemy(cfg) {}

		/// @brief Executed every update cycle.
		void onUpdate(float delta) override {
			if (!active) return;
			AEnemy::onUpdate(delta);
			if (paused()) return;
		}

		/// @brief Executed every update cycle.
		void onUpdate(float delta, Makai::App& app) override {
			if (!active) return;
			AEnemy::onUpdate(delta, app);
			if (paused()) return;
			if (currentAct) currentAct->onUpdate(delta, app);
		}

		/// @brief Called when the enemy dies. In this case, when a phase ends.
		void onDeath() override {
			if (!practiceMode && currentAct && (currentAct = onAct(currentAct->next())))
				setFlags(Flags::EF_DEAD, false);
			else endBattle();
		}

		/// @brief Begins the boss battle.
		/// @return Reference to self.
		ABoss& beginBattle() {
			currentAct.unbind();
			onBattleBegin();
			return *this;
		}

		/// @brief Ends the boss battle.
		/// @return Reference to self.
		ABoss& endBattle() {
			currentAct.unbind();
			onBattleEnd();
			return *this;
		}

		/// @brief Executes a specific act.
		/// @return Reference to self.
		ABoss& doAct(usize const act, bool const practice = false) {
			practiceMode = practice;
			currentAct = onAct(act);
			return *this;
		}

		/// @brief Whether the boss is in practice mode.
		bool practiceMode = false;

	protected:
		/// @brief Returns the current act.
		/// @return Reference to current act.
		Reference<AAct> getCurrentAct() const {
			return currentAct.reference();
		}

		/// @brief Called when battle begins. Must be implemented.
		virtual void onBattleBegin()					= 0;
		/// @brief Called when a specific act is requested. Must be implemented.
		virtual ActInstanceType onAct(usize const act)	= 0;
		/// @brief Called when battle ends. Must be implemented.
		virtual void onBattleEnd()						= 0;

	private:
		/// @brief Current act being executed.
		Unique<AAct> currentAct;
	};
}

#endif