export type { SenseNodeDataSource } from '../../shared/types/sensenode'

export class WifiDataSource {
	readonly type = 'wifi'

	async connect(): Promise<void> {
		throw new Error('WiFi data source is reserved for the next phase')
	}

	async disconnect(): Promise<void> {
		return Promise.resolve()
	}
}
