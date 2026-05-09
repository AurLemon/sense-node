import { defineStore } from 'pinia'
import { computed, ref } from 'vue'
import type {
	DataSourceType,
	SenseNodeEventLog,
	SenseNodeFrame,
	StableEvent,
	StabilizerSnapshot,
} from '../../shared/types/sensenode'
import { EventStabilizer } from '../lib/eventStabilizer'

const stabilizer = new EventStabilizer()
const frameRetentionMs = 5 * 60 * 1000
const maxRetainedFrames = 6000

export const useDeviceStore = defineStore('device', () => {
	const currentFrame = ref<SenseNodeFrame | null>(null)
	const frames = ref<SenseNodeFrame[]>([])
	const events = ref<SenseNodeEventLog[]>([])
	const persistedEvents = ref<SenseNodeEventLog[]>([])
	const stabilizerSnapshot = ref<StabilizerSnapshot>(stabilizer.getSnapshot())
	const source = ref<DataSourceType | null>(null)
	const stabilizerEnabled = ref(true)

	const stableEvent = computed<StableEvent>(
		() => stabilizerSnapshot.value.stableEvent,
	)
	const rawEvent = computed(() => stabilizerSnapshot.value.rawEvent)

	function ingestFrame(
		frame: SenseNodeFrame,
		nextSource: DataSourceType,
	): void {
		source.value = nextSource
		const normalized = {
			...frame,
			timestamp: frame.timestamp ?? Date.now(),
		}
		currentFrame.value = normalized
		frames.value.unshift(normalized)
		const retentionStart = normalized.timestamp - frameRetentionMs
		frames.value = frames.value.filter(
			(item) => (item.timestamp ?? 0) >= retentionStart,
		)
		if (frames.value.length > maxRetainedFrames) {
			frames.value.length = maxRetainedFrames
		}

		if (normalized.type !== 'sensor_frame') {
			return
		}

		stabilizerSnapshot.value = stabilizerEnabled.value
			? stabilizer.update(normalized)
			: {
					rawEvent: (normalized.final_event as StableEvent) ?? 'unknown',
					stableEvent: (normalized.final_event as StableEvent) ?? 'unknown',
					pendingEvent: null,
					confidence: normalized.confidence ?? 1,
					state: 'stable',
					lastTransitionAt: Date.now(),
					holdUntil: 0,
					reason: 'stabilizer disabled',
				}

		const log: SenseNodeEventLog = {
			id: `${Date.now()}-${Math.random().toString(16).slice(2)}`,
			timestamp: normalized.timestamp,
			source: nextSource,
			rawEvent: stabilizerSnapshot.value.rawEvent,
			stableEvent: stabilizerSnapshot.value.stableEvent,
			confidence: normalized.confidence,
			tofMm: normalized.tof_mm,
			inferenceMs: normalized.inference_ms,
			frame: normalized,
		}
		events.value.unshift(log)
		if (events.value.length > 100) {
			events.value.length = 100
		}
		persistedEvents.value.unshift(log)
		if (persistedEvents.value.length > 200) {
			persistedEvents.value.length = 200
		}
		void window.sensenode.events.addLog(log)
	}

	async function loadPersistedEvents(limit = 200): Promise<void> {
		persistedEvents.value = await window.sensenode.events.listLogs(limit)
	}

	return {
		currentFrame,
		frames,
		events,
		persistedEvents,
		stabilizerSnapshot,
		source,
		stabilizerEnabled,
		stableEvent,
		rawEvent,
		ingestFrame,
		loadPersistedEvents,
	}
})
