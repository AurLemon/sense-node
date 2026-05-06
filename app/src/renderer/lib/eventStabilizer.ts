import type {
	SenseNodeFrame,
	StableEvent,
	StabilizerSnapshot,
} from '../../shared/types/sensenode'

const knownEvents: StableEvent[] = [
	'idle',
	'tap',
	'board_motion',
	'hand_hover',
	'hand_near',
	'hand_leave',
	'reject',
	'unknown',
]

export interface EventStabilizerOptions {
	minConfidence: number
	boardMotionFrames: number
	idleMs: number
	tapHoldMs: number
	cooldownMs: number
}

const defaultOptions: EventStabilizerOptions = {
	minConfidence: 0.58,
	boardMotionFrames: 3,
	idleMs: 900,
	tapHoldMs: 450,
	cooldownMs: 700,
}

export class EventStabilizer {
	private stableEvent: StableEvent = 'idle'
	private pendingEvent: StableEvent | null = null
	private pendingSince = 0
	private boardMotionCount = 0
	private lastTransitionAt = Date.now()
	private holdUntil = 0
	private cooldownUntil = 0
	private snapshot: StabilizerSnapshot = {
		rawEvent: 'idle',
		stableEvent: 'idle',
		pendingEvent: null,
		confidence: 1,
		state: 'stable',
		lastTransitionAt: Date.now(),
		holdUntil: 0,
		reason: 'initial',
	}

	constructor(private readonly options = defaultOptions) {}

	update(frame: SenseNodeFrame, now = Date.now()): StabilizerSnapshot {
		const rawEvent = normalizeEvent(
			frame.final_event ?? frame.motion_event ?? frame.hand_state,
		)
		const confidence = frame.confidence ?? 1
		let reason = 'stable hold'
		let state: StabilizerSnapshot['state'] = 'stable'

		if (now < this.holdUntil) {
			state = 'pulse'
			reason = 'momentary event hold'
			return this.capture(rawEvent, confidence, state, reason)
		}

		if (confidence < this.options.minConfidence && rawEvent !== 'tap') {
			this.pendingEvent = rawEvent
			state = 'pending'
			reason = 'low confidence pending'
			return this.capture(rawEvent, confidence, state, reason)
		}

		if (rawEvent === 'tap') {
			this.transition('tap', now)
			this.holdUntil = now + this.options.tapHoldMs
			this.cooldownUntil = now + this.options.cooldownMs
			return this.capture(rawEvent, confidence, 'pulse', 'tap pulse')
		}

		if (now < this.cooldownUntil && rawEvent !== this.stableEvent) {
			return this.capture(
				rawEvent,
				confidence,
				'cooldown',
				'transition cooldown',
			)
		}

		if (rawEvent === 'board_motion') {
			this.boardMotionCount += 1
			if (this.boardMotionCount >= this.options.boardMotionFrames) {
				this.transition('board_motion', now)
				this.cooldownUntil = now + this.options.cooldownMs
				reason = 'board motion confirmed'
			} else {
				this.pendingEvent = 'board_motion'
				state = 'pending'
				reason = 'board motion debounce'
			}
			return this.capture(rawEvent, confidence, state, reason)
		}

		this.boardMotionCount = 0

		if (rawEvent === 'idle' && this.stableEvent === 'board_motion') {
			if (this.pendingEvent !== 'idle') {
				this.pendingEvent = 'idle'
				this.pendingSince = now
			}
			if (now - this.pendingSince >= this.options.idleMs) {
				this.transition('idle', now)
				reason = 'idle decay confirmed'
			} else {
				state = 'pending'
				reason = 'idle decay'
			}
			return this.capture(rawEvent, confidence, state, reason)
		}

		if (rawEvent !== this.stableEvent) {
			this.transition(rawEvent, now)
			reason = 'direct stable event'
		}

		return this.capture(rawEvent, confidence, state, reason)
	}

	getSnapshot(): StabilizerSnapshot {
		return this.snapshot
	}

	private transition(event: StableEvent, now: number): void {
		this.stableEvent = event
		this.pendingEvent = null
		this.pendingSince = 0
		this.lastTransitionAt = now
	}

	private capture(
		rawEvent: StableEvent,
		confidence: number,
		state: StabilizerSnapshot['state'],
		reason: string,
	): StabilizerSnapshot {
		this.snapshot = {
			rawEvent,
			stableEvent: this.stableEvent,
			pendingEvent: this.pendingEvent,
			confidence,
			state,
			lastTransitionAt: this.lastTransitionAt,
			holdUntil: this.holdUntil,
			reason,
		}
		return this.snapshot
	}
}

export function normalizeEvent(value?: string): StableEvent {
	if (!value) {
		return 'unknown'
	}
	const normalized = value.trim().toLowerCase() as StableEvent
	return knownEvents.includes(normalized) ? normalized : 'unknown'
}
