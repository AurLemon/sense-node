import { Notification } from 'electron'
import { spawn } from 'node:child_process'
import type { EventTask } from '../../shared/types/sensenode'

export function executeEventTask(task: EventTask, eventLabel: string): void {
	const title = task.name
	const body = task.action.message || eventLabel

	if (task.action.type === 'notify') {
		if (Notification.isSupported()) {
			new Notification({
				title,
				body: task.action.title ? `${task.action.title}\n${body}` : body,
			}).show()
		}
		return
	}

	const command = task.action.command?.trim()
	if (!command) return

	const args = task.action.arguments ?? []
	if (task.action.type === 'run_exe') {
		spawn(command, args, {
			detached: true,
			stdio: 'ignore',
			windowsHide: true,
		}).unref()
		return
	}

	spawn(command, args, {
		detached: true,
		stdio: 'ignore',
		windowsHide: true,
		shell: true,
	}).unref()
}
