<script setup lang="ts">
import { computed, reactive, ref, toRaw, watch } from 'vue'
import { useToast } from '@nuxt/ui/composables'
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'
import { useEventStore } from '../../stores/eventStore'
import { stableEvents } from '../../../shared/types/sensenode'
import type {
	EventActionType,
	EventTask,
	StableEvent,
} from '../../../shared/types/sensenode'

const device = useDeviceStore()
const events = useEventStore()
const toast = useToast()
const { t } = useI18n()
const modalOpen = ref(false)
const editingId = ref<string | null>(null)

const modalUi = {
	overlay: 'fixed inset-0 z-[110] bg-default/65',
	content: 'z-[120] max-w-lg',
	footer: 'justify-end',
}

const selectUi = {
	content: 'z-[130]',
}

const draft = reactive<EventTask>(createDraft())
const argsText = ref('')
const isSaving = ref(false)

const actionOptions = computed<
	Array<{ value: EventActionType; label: string }>
>(() => [
	{ value: 'notify', label: t('events.task.notifyAction') },
	{ value: 'run_exe', label: t('events.task.runExe') },
	{ value: 'run_script', label: t('events.task.runScript') },
])

const eventOptions = computed(() =>
	stableEvents.map((event) => ({
		value: event,
		label: t(`events.event.${event}`),
	})),
)

function createDraft(): EventTask {
	const now = Date.now()
	return {
		id: '',
		name: '',
		enabled: true,
		event: 'tap',
		cooldownMs: 2000,
		action: {
			type: 'notify',
			title: '',
			message: '',
			command: '',
			arguments: [],
		},
		createdAt: now,
		updatedAt: now,
	}
}

function validateDraft(): string | null {
	if (!draft.name.trim()) return t('events.task.name')
	if (!draft.event) return t('events.task.event')
	if (!draft.action.type) return t('events.task.action')
	if (!draft.action.title.trim()) return t('events.task.notifyTitle')
	if (!draft.action.message.trim()) return t('events.task.notifyBody')
	if (
		!Number.isFinite(Number(draft.cooldownMs)) ||
		Number(draft.cooldownMs) < 0
	) {
		return t('events.task.cooldown')
	}

	if (
		(draft.action.type === 'run_exe' || draft.action.type === 'run_script') &&
		!draft.action.command?.trim()
	) {
		return t('events.task.command')
	}

	return null
}

function startCreate(): void {
	editingId.value = null
	Object.assign(draft, createDraft())
	argsText.value = ''
	modalOpen.value = true
}

function editTask(task: EventTask): void {
	try {
		editingId.value = task.id
		Object.assign(draft, structuredClone(toRaw(task)))
		argsText.value = task.action.arguments?.join(', ') ?? ''
		modalOpen.value = true
	} catch (error) {
		console.error('Failed to open event task editor:', error)
		const detail =
			error instanceof Error
				? error.message
				: typeof error === 'string'
					? error
					: ''
		toast.add({
			title: detail
				? `${t('events.task.saveFailed')}: ${detail}`
				: t('events.task.saveFailed'),
			color: 'error',
		})
	}
}

function buildPayload(): EventTask {
	const now = Date.now()
	const actionType = draft.action.type
	return {
		id: editingId.value ?? (draft.id || crypto.randomUUID()),
		name: draft.name.trim(),
		enabled: Boolean(draft.enabled),
		event: draft.event,
		cooldownMs: Number(draft.cooldownMs) || 0,
		createdAt: editingId.value ? draft.createdAt : now,
		updatedAt: now,
		action: {
			type: actionType,
			title: draft.action.title.trim(),
			message: draft.action.message.trim(),
			command: draft.action.command?.trim() || undefined,
			arguments: argsText.value
				.split(',')
				.map((item) => item.trim())
				.filter(Boolean),
		},
	}
}

async function save(): Promise<void> {
	if (isSaving.value) return
	const missingField = validateDraft()
	if (missingField) {
		toast.add({
			title: t('events.task.invalid'),
			description: missingField,
			color: 'warning',
		})
		return
	}

	isSaving.value = true
	try {
		await events.saveTask(buildPayload())
		toast.add({ title: t('events.task.saved'), color: 'success' })
		modalOpen.value = false
	} catch (error) {
		console.error('Failed to save event task:', error)
		const detail =
			error instanceof Error
				? error.message
				: typeof error === 'string'
					? error
					: ''
		toast.add({
			title: detail
				? `${t('events.task.saveFailed')}: ${detail}`
				: t('events.task.saveFailed'),
			color: 'error',
		})
	} finally {
		isSaving.value = false
	}
}

async function removeTask(id: string): Promise<void> {
	try {
		await events.removeTask(id)
		toast.add({ title: t('events.task.deleted'), color: 'success' })
		if (editingId.value === id) modalOpen.value = false
	} catch (error) {
		console.error('Failed to remove event task:', error)
		const detail =
			error instanceof Error
				? error.message
				: typeof error === 'string'
					? error
					: ''
		toast.add({
			title: detail
				? `${t('events.task.saveFailed')}: ${detail}`
				: t('events.task.saveFailed'),
			color: 'error',
		})
	}
}

watch(
	() => device.stableEvent,
	(event) => {
		events.activeEvent = event
	},
	{ immediate: true },
)

if (events.tasks.length === 0) {
	void events.loadTasks()
}
</script>

<template>
	<div class="grid gap-3">
		<UTooltip :text="t('events.task.add')">
			<UButton
				class="justify-self-start"
				icon="i-lucide-plus"
				color="neutral"
				variant="ghost"
				size="sm"
				type="button"
				:aria-label="t('events.task.add')"
				@click="startCreate"
			/>
		</UTooltip>

		<div
			v-if="events.tasks.length === 0"
			class="py-8 text-center text-sm text-muted"
		>
			{{ t('events.task.empty') }}
		</div>

		<div v-else class="grid gap-2">
			<article
				v-for="task in events.tasks"
				:key="task.id"
				class="grid gap-2 rounded-lg border border-default bg-default/75 p-2.5 backdrop-blur"
			>
				<div class="flex items-start justify-between gap-3">
					<div class="min-w-0">
						<div class="flex min-w-0 items-center gap-2">
							<h3 class="truncate text-sm font-medium">
								{{ task.name }}
							</h3>
							<UBadge
								:color="task.enabled ? 'success' : 'neutral'"
								variant="soft"
							>
								{{ t('events.task.enabled') }}
							</UBadge>
						</div>
						<div class="mt-1 flex flex-wrap items-center gap-1.5">
							<UBadge color="neutral" variant="soft">
								{{ t(`events.event.${task.event}`) }}
							</UBadge>
							<UBadge color="neutral" variant="outline">
								{{ task.action.type }}
							</UBadge>
							<UBadge color="neutral" variant="subtle">
								{{ task.cooldownMs }}ms
							</UBadge>
						</div>
					</div>

					<div class="flex shrink-0 items-center gap-1">
						<UTooltip :text="t('common.apply')">
							<UButton
								icon="i-lucide-pencil"
								color="neutral"
								variant="ghost"
								size="xs"
								type="button"
								:aria-label="t('common.apply')"
								@click="editTask(task)"
							/>
						</UTooltip>
						<UTooltip :text="t('events.task.remove')">
							<UButton
								icon="i-lucide-trash-2"
								color="error"
								variant="ghost"
								size="xs"
								type="button"
								:aria-label="t('events.task.remove')"
								@click="removeTask(task.id)"
							/>
						</UTooltip>
					</div>
				</div>

				<p class="truncate text-sm text-muted">
					{{ task.action.title || task.action.message || task.action.command }}
				</p>
			</article>
		</div>

		<UModal
			v-model:open="modalOpen"
			:title="editingId ? t('events.task.save') : t('events.task.add')"
			:overlay="true"
			:portal="true"
			:ui="modalUi"
		>
			<template #body>
				<div class="grid gap-3">
					<UFormField :label="t('events.task.name')" required>
						<UInput v-model="draft.name" size="sm" class="w-full" required />
					</UFormField>

					<UFormField :label="t('events.task.event')" required>
						<USelect
							v-model="draft.event"
							:items="eventOptions"
							value-key="value"
							label-key="label"
							:ui="selectUi"
							class="w-full"
						/>
					</UFormField>

					<UFormField :label="t('events.task.action')" required>
						<USelect
							v-model="draft.action.type"
							:items="actionOptions"
							value-key="value"
							label-key="label"
							:ui="selectUi"
							class="w-full"
						/>
					</UFormField>

					<UFormField :label="t('events.task.notifyTitle')" required>
						<UInput
							v-model="draft.action.title"
							size="sm"
							class="w-full"
							:placeholder="t('events.task.notifyTitlePlaceholder')"
						/>
					</UFormField>

					<UFormField :label="t('events.task.notifyBody')" required>
						<UTextarea
							v-model="draft.action.message"
							:rows="3"
							size="sm"
							class="w-full"
							:placeholder="t('events.task.notifyBodyPlaceholder')"
						/>
					</UFormField>

					<UFormField :label="t('events.task.command')">
						<UInput
							v-model="draft.action.command"
							size="sm"
							class="w-full"
							:placeholder="
								draft.action.type === 'run_exe'
									? t('events.task.exeHint')
									: t('events.task.scriptHint')
							"
						/>
					</UFormField>

					<UFormField :label="t('events.task.args')">
						<UInput
							v-model="argsText"
							size="sm"
							class="w-full"
							:placeholder="t('events.task.argHint')"
						/>
					</UFormField>

					<UFormField :label="t('events.task.cooldown')" required>
						<UInput
							v-model="draft.cooldownMs"
							size="sm"
							type="number"
							:min="0"
							class="w-full"
						/>
					</UFormField>

					<label
						class="grid min-h-8 grid-cols-[auto_1fr] items-center gap-2 text-sm text-default"
					>
						<UCheckbox v-model="draft.enabled" />
						<span>{{ t('events.task.enabled') }}</span>
					</label>
				</div>
			</template>

			<template #footer>
				<UButton
					color="neutral"
					variant="ghost"
					type="button"
					@click="modalOpen = false"
				>
					{{ t('common.close') }}
				</UButton>
				<UButton
					icon="i-lucide-save"
					color="primary"
					variant="solid"
					type="button"
					:loading="isSaving"
					@click="save"
				>
					{{ t('common.save') }}
				</UButton>
			</template>
		</UModal>
	</div>
</template>
