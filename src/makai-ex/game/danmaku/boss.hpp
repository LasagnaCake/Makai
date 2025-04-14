#ifndef MAKAILIB_EX_GAME_DANMAKU_BOSS_H
#define MAKAILIB_EX_GAME_DANMAKU_BOSS_H

#include "enemy.hpp"

/// @brief Danmaku-specific game extensions.
namespace Makai::Ex::Game::Danmaku {
	/// @brief Boss enemy.
	struct ABoss: AEnemy {
		/// @brief Constructs the boss.
		/// @param cfg Enemy configuration.
		ABoss(EnemyConfig const& cfg): AEnemy(cfg) {}

		/// @brief Executed every update cycle.
		void onUpdate(float delta) override {
			if (!active) return;
			AEnemy::onUpdate(delta);
			if (paused()) return;
		}

		/// @brief Called when the enemy dies. In this case, when a phase ends.
		void onDeath() override {
			if (++currentAct < getActCount() && !practiceMode) {
				setFlags(Flags::EF_DEAD, false);
				doCurrentAct();
			} else onBattleEnd();
		}

		/// @brief Begins the boss battle.
		/// @return Reference to self.
		ABoss& beginBattle() {
			currentAct = 0;
			onBattleBegin();
			return *this;
		}

		/// @brief Executes a specific act, independent of which act is currently selected.
		/// @return Reference to self.
		ABoss& doAct(usize const act, bool const practice = false) {
			practiceMode = practice;
			onAct(act);
			return *this;
		}
		
		/// @brief Executes the currently-selected act.
		/// @return Reference to self.
		ABoss& doCurrentAct() {
			onAct(currentAct);
			return *this;
		}

		/// @brief Sets the currently-selected act.
		/// @param act Act to set.
		/// @return Reference to self.
		ABoss& setAct(usize const act) {
			auto const total = getActCount();
			currentAct = act < total ? act : total;
			return *this;
		}

		/// @brief Returns the total act count. Must be implemented.
		virtual usize getActCount()	= 0;

		/// @brief Whether the boss is in practice mode.
		bool practiceMode = false;

	protected:
		/// @brief Called when battle begins. Must be implemented.
		virtual void onBattleBegin()		= 0;
		/// @brief Called when a specific act is requested. Must be implemented.
		virtual void onAct(usize const act)	= 0;
		/// @brief Called when battle ends. Must be implemented.
		virtual void onBattleEnd()			= 0;

	private:
		/// @brief Currently-selected act.
		usize currentAct = 0;
	};
}

#endif