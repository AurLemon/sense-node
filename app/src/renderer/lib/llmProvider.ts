import type {
	LlmConfig,
	SenseNodeEventLog,
	SenseNodeFrame,
	StableEvent,
} from '../../shared/types/sensenode'

export interface LlmAdapter {
	analyze(input: {
		recentFrames: SenseNodeFrame[]
		recentEvents: SenseNodeEventLog[]
		currentState: StableEvent
	}): Promise<string>
}

export function createLlmAdapter(config: LlmConfig): LlmAdapter {
	return {
		async analyze() {
			return `LLM provider "${config.provider}" is reserved for a later phase.`
		},
	}
}
