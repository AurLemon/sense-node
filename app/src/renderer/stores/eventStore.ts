import { defineStore } from 'pinia'
import { computed, ref } from 'vue'
import type { EventTask, StableEvent } from '../../shared/types/sensenode'

export const useEventStore = defineStore('event', () => {
	const tasks = ref<EventTask[]>([])
	const activeEvent = ref<StableEvent>('idle')

	const tasksByEvent = computed(() =>
		tasks.value.reduce<Record<StableEvent, EventTask[]>>(
			(acc, task) => {
				;(acc[task.event] ??= []).push(task)
				return acc
			},
			{
				idle: [],
				tap: [],
				board_motion: [],
				hand_hover: [],
				hand_near: [],
				hand_leave: [],
				reject: [],
				unknown: [],
			},
		),
	)

	async function loadTasks(): Promise<void> {
		tasks.value = await window.sensenode.events.listTasks()
	}

	async function saveTask(task: EventTask): Promise<void> {
		const saved = await window.sensenode.events.saveTask(task)
		const index = tasks.value.findIndex((item) => item.id === saved.id)
		if (index >= 0) {
			tasks.value[index] = saved
		} else {
			tasks.value.unshift(saved)
		}
	}

	async function removeTask(id: string): Promise<void> {
		await window.sensenode.events.removeTask(id)
		tasks.value = tasks.value.filter((task) => task.id !== id)
	}

	async function notifyStableEvent(event: StableEvent): Promise<void> {
		activeEvent.value = event
		await window.sensenode.events.notifyStableEvent(event)
	}

	return {
		tasks,
		tasksByEvent,
		activeEvent,
		loadTasks,
		saveTask,
		removeTask,
		notifyStableEvent,
	}
})
