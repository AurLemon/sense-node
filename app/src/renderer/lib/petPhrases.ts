import type { Locale } from '../../shared/i18n'

type PhraseEvent = 'idle' | 'tap' | 'board_motion'

const phraseBuckets: Record<Locale, Record<PhraseEvent, string[]>> = {
	'zh-CN': {
		idle: [
			'看什么看 我在待机',
			'今天也在认真装可爱',
			'别戳我 我会记仇',
			'我很乖 但只乖五秒',
			'摸鱼中 勿扰',
			'你不动 我也不动',
			'刚才那个姿势不错',
			'我只是个小板子 别太指望我',
			'再看我就卖萌了',
			'嗯 是在叫我吗',
		],
		tap: [
			'别拍我',
			'轻点 我有脾气',
			'谁打我小脑袋',
			'收到 但我不一定听',
			'你礼貌吗',
			'啪一下很开心吗',
			'我醒着呢 别敲了',
			'再拍就装死',
			'哎呀 手滑也算你打我',
			'有事说事 别动手',
		],
		board_motion: [
			'晃晃悠悠 像在上班',
			'别摇了 我要晕板了',
			'这就是传说中的位移吗',
			'我动了 但我不承认',
			'慢点 传感器都看着呢',
			'你拿起我的样子很狼狈',
			'正在优雅地漂移',
			'这段运动我给三分',
			'别晃 我在思考人生',
			'板子移动中 请保持体面',
		],
	},
	'en-US': {
		idle: [
			'Staring contest I win',
			'Idle but make it cute',
			'Do not poke the tiny board',
			'I am behaving for now',
			'Quietly judging your posture',
			'No motion no drama',
			'I am not asleep probably',
			'Small board big attitude',
			'Blink if you need snacks',
			'Still here still adorable',
		],
		tap: [
			'Hey do not slap me',
			'Gentle taps only boss',
			'That was my forehead',
			'I heard you unfortunately',
			'Rude but acknowledged',
			'Tap received grudge stored',
			'I was awake already',
			'Again and I play dead',
			'Use words not violence',
			'That counted as bonk',
		],
		board_motion: [
			'Motion detected dignity lost',
			'Please stop shaking the intern',
			'I am moving allegedly',
			'That was not graceful',
			'Sensors saw everything',
			'Careful I get board sick',
			'Drifting with questionable style',
			'Movement score three out of ten',
			'I am trying to think here',
			'Board in transit stay classy',
		],
	},
}

export function pickPetPhrase(
	locale: Locale,
	finalEvent: string | undefined,
	previous?: string,
): string {
	const bucket = phraseBuckets[locale][normalizePhraseEvent(finalEvent)]
	if (bucket.length <= 1) {
		return bucket[0] ?? ''
	}

	let next = bucket[Math.floor(Math.random() * bucket.length)]
	if (next === previous) {
		next = bucket[(bucket.indexOf(next) + 1) % bucket.length]
	}
	return next
}

function normalizePhraseEvent(finalEvent: string | undefined): PhraseEvent {
	if (finalEvent === 'tap') {
		return 'tap'
	}
	if (finalEvent === 'board_motion') {
		return 'board_motion'
	}
	return 'idle'
}
