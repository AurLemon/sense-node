<script setup lang="ts">
import { computed, ref, watch } from 'vue'
import { useToast } from '@nuxt/ui/composables'
import { useI18n } from '../../lib/i18n'
import { useSerialStore } from '../../stores/serialStore'

const serial = useSerialStore()
const { t } = useI18n()
const toast = useToast()
const settingsOpen = ref(false)
const discardConfirmOpen = ref(false)
const portsOpen = ref(false)
const baudRateDraft = ref(serial.baudRate)

const portItems = computed(() =>
	serial.ports.map((port) => {
		const parts = [port.manufacturer, port.friendlyName].filter(Boolean)

		return {
			value: port.path,
			label: port.path,
			description: parts.length > 0 ? parts.join(' · ') : t('serial.autoPick'),
		}
	}),
)

const isBusy = computed(() =>
	['scanning', 'connecting', 'disconnecting', 'reconnecting'].includes(
		serial.status.state,
	),
)

const isConnected = computed(() => serial.status.state === 'connected')
const selectedPortModel = computed(() =>
	isConnected.value ? (serial.status.selectedPort ?? serial.selectedPort) : '',
)
const connectionActionLabel = computed(() =>
	isConnected.value ? t('common.reconnect') : t('common.connect'),
)
const hasUnsavedSettings = computed(
	() => Number(baudRateDraft.value) !== serial.baudRate,
)

const modalOpen = computed({
	get: () => settingsOpen.value,
	set: (open) => {
		if (!open && hasUnsavedSettings.value) {
			discardConfirmOpen.value = true
			return
		}
		settingsOpen.value = open
	},
})

watch(settingsOpen, (open) => {
	if (open) {
		baudRateDraft.value = serial.baudRate
	}
})

watch(portsOpen, (open) => {
	if (open) {
		void serial.refreshPorts()
	}
})

async function selectAndConnect(portPath: string): Promise<void> {
	const connected = await serial.connectAndValidate(portPath)
	if (!connected) {
		showConnectFailedToast()
	}
}

async function connect(): Promise<void> {
	const connected = await serial.connectAndValidate()
	if (!connected) {
		showConnectFailedToast()
	}
}

async function disconnect(): Promise<void> {
	await serial.disconnect()
}

function saveSettings(): void {
	serial.baudRate = Number(baudRateDraft.value)
	settingsOpen.value = false
}

function discardSettings(): void {
	baudRateDraft.value = serial.baudRate
	discardConfirmOpen.value = false
	settingsOpen.value = false
}

function showConnectFailedToast(): void {
	toast.add({
		title: t('serial.connectFailed'),
		description: t('serial.connectFailedDescription'),
		icon: 'i-lucide-circle-alert',
		color: 'warning',
	})
}
</script>

<template>
	<div class="flex items-center justify-between gap-3">
		<div class="flex min-w-0 items-center gap-1.5">
			<USelectMenu
				v-model:open="portsOpen"
				:model-value="selectedPortModel"
				:items="portItems"
				value-key="value"
				label-key="label"
				description-key="description"
				:placeholder="t('serial.selectPort')"
				:search-input="{
					placeholder: t('serial.searchPort'),
					icon: 'i-lucide-search',
				}"
				class="w-48 min-w-0"
				:disabled="isBusy"
				@update:model-value="selectAndConnect"
			/>
			<UTooltip :text="connectionActionLabel">
				<UButton
					icon="i-lucide-plug-zap"
					color="neutral"
					variant="ghost"
					size="sm"
					type="button"
					:aria-label="connectionActionLabel"
					:disabled="!serial.selectedPort || isBusy"
					:loading="serial.status.state === 'connecting'"
					@click="connect"
				/>
			</UTooltip>
			<UTooltip :text="t('common.disconnect')">
				<UButton
					icon="i-lucide-unplug"
					color="neutral"
					variant="ghost"
					size="sm"
					type="button"
					:aria-label="t('common.disconnect')"
					:disabled="!isConnected || isBusy"
					@click="disconnect"
				/>
			</UTooltip>
		</div>

		<div class="flex items-center">
			<UModal
				v-model:open="modalOpen"
				:title="t('serial.connectionSettings')"
				:ui="{ content: 'max-w-sm', footer: 'justify-end' }"
			>
				<UTooltip :text="t('common.settings')">
					<UButton
						icon="i-lucide-settings"
						color="neutral"
						variant="ghost"
						size="sm"
						type="button"
						:aria-label="t('common.settings')"
					/>
				</UTooltip>

				<template #body>
					<div class="grid gap-3">
						<UFormField :label="t('serial.baudRate')">
							<UInput
								v-model="baudRateDraft"
								type="number"
								:min="9600"
								class="w-full"
							/>
						</UFormField>
					</div>
				</template>

				<template #footer>
					<UButton
						color="primary"
						variant="solid"
						type="button"
						@click="saveSettings"
					>
						{{ t('common.save') }}
					</UButton>
				</template>
			</UModal>

			<UModal
				v-model:open="discardConfirmOpen"
				:title="t('serial.unsavedSettingsTitle')"
				:ui="{ content: 'max-w-sm', footer: 'justify-end' }"
			>
				<template #body>
					<p class="text-sm text-muted">
						{{ t('serial.unsavedSettingsDescription') }}
					</p>
				</template>

				<template #footer>
					<UButton
						color="neutral"
						variant="ghost"
						type="button"
						@click="discardConfirmOpen = false"
					>
						{{ t('common.cancel') }}
					</UButton>
					<UButton
						color="primary"
						variant="solid"
						type="button"
						@click="discardSettings"
					>
						{{ t('common.discard') }}
					</UButton>
				</template>
			</UModal>
		</div>
	</div>
</template>
