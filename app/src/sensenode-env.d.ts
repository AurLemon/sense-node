declare global {
	interface Window {
		sensenode: {
			serial: {
				listPorts(): Promise<
					import('./shared/types/sensenode').SerialPortInfo[]
				>
				connect(
					path: string,
					baudRate?: number,
				): Promise<import('./shared/types/sensenode').DataSourceStatus>
				disconnect(): Promise<
					import('./shared/types/sensenode').DataSourceStatus
				>
				pause(): Promise<import('./shared/types/sensenode').DataSourceStatus>
				resume(): Promise<import('./shared/types/sensenode').DataSourceStatus>
				onLine(
					callback: (
						line: import('./shared/types/sensenode').SerialLineEntry,
					) => void,
				): () => void
			}
			device: {
				onFrame(
					callback: (
						frame: import('./shared/types/sensenode').SenseNodeFrame,
					) => void,
				): () => void
				onStatus(
					callback: (
						status: import('./shared/types/sensenode').DataSourceStatus,
					) => void,
				): () => void
			}
			app: {
				showMainWindow(): Promise<void>
				showHud(): Promise<void>
				hideHud(): Promise<void>
				setLocale(locale: string): Promise<void>
			}
		}
	}
}

export {}
