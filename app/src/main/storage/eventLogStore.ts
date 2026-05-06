import type { SenseNodeEventLog } from '../../shared/types/sensenode'

export class EventLogStore {
	private readonly events: SenseNodeEventLog[] = []
	private readonly limit = 500

	add(event: SenseNodeEventLog): void {
		this.events.unshift(event)
		if (this.events.length > this.limit) {
			this.events.length = this.limit
		}
	}

	list(): SenseNodeEventLog[] {
		return [...this.events]
	}
}
