#ifndef MAKAILIB_EX_GAME_DANMAKU_BOSS_H
#define MAKAILIB_EX_GAME_DANMAKU_BOSS_H

#include "enemy.hpp"

namespace Makai::Ex::Game::Danmaku {
	struct ABoss: AEnemy {
		ABoss(EnemyConfig const& cfg): AEnemy(cfg) {}

		void onUpdate(float delta) override {
			if (!active) return;
			AEnemy::onUpdate(delta);
			if (paused()) return;
		}

		void onDeath() override {
			if (currentAct++ < getActCount() && !practiceMode) {
				setFlags(DEAD, false);
				doCurrentAct();
			} else onBattleEnd();
		}

		ABoss& beginBattle() {
			currentAct = 0;
			onBattleBegin();
			return *this;
		}

		ABoss& doAct(usize const act, bool const practice = false) {
			practiceMode = practice;
			onAct(act);
			return *this;
		}

		ABoss& doCurrentAct() {
			onAct(currentAct);
			return *this;
		}

		ABoss& setAct(usize const act) {
			auto const total = getActCount();
			currentAct = act < total ? act : total;
			return *this;
		}

		virtual usize getActCount()	= 0;

		bool practiceMode = false;

	protected:
		virtual void onBattleBegin()		= 0;
		virtual void onAct(usize const act)	= 0;
		virtual void onBattleEnd()			= 0;

	private:
		usize currentAct = 0;
	};
}

#endif