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
				setLocale(locale: string): Promise<void>
			}
			events: {
				notifyStableEvent(event: string): Promise<void>
				listLogs(
					limit?: number,
				): Promise<import('./shared/types/sensenode').SenseNodeEventLog[]>
				addLog(
					log: import('./shared/types/sensenode').SenseNodeEventLog,
				): Promise<import('./shared/types/sensenode').SenseNodeEventLog>
				listTasks(): Promise<import('./shared/types/sensenode').EventTask[]>
				saveTask(
					task: import('./shared/types/sensenode').EventTask,
				): Promise<import('./shared/types/sensenode').EventTask>
				removeTask(id: string): Promise<void>
			}
		}
	}
}

export {}
