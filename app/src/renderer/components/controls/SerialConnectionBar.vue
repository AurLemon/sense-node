<script setup lang="ts">
import { computed, ref } from 'vue'
import { getTransportLabel, useI18n } from '../../lib/i18n'
import { useSerialStore } from '../../stores/serialStore'

const serial = useSerialStore()
const { t, locale } = useI18n()

const portItems = computed(() =>
	serial.ports.map((port) => {
		const transportLabel = getTransportLabel(locale.value, port.transport)
		const parts = [port.manufacturer, port.friendlyName].filter(Boolean)

		return {
			value: port.path,
			label: port.path,
			description: parts.length > 0 ? parts.join(' · ') : t('serial.autoPick'),
			chip: {
				text: port.isLikelyDevice
					? `${transportLabel} · ${t('serial.likely')}`
					: transportLabel,
				color: 'neutral',
				size: 'sm',
			},
		}
	}),
)

const stateLabel = computed(() => t(`serial.status.${serial.status.state}`))
const selectedLabel = computed(
	() =>
		serial.ports.find((port) => port.path === serial.selectedPort)?.path ??
		serial.selectedPort ??
		t('common.none'),
)
const baudRate = computed({
	get: () => serial.baudRate,
	set: (value: number | string) => {
		serial.baudRate = Number(value)
	},
})

async function refresh(): Promise<void> {
	await serial.refreshPorts()
}

async function connect(): Promise<void> {
	await serial.connect()
}

async function disconnect(): Promise<void> {
	await serial.disconnect()
}
</script>

<template>
	<div
		class="grid grid-cols-[minmax(0,1fr)_140px_auto] gap-2.5 rounded-lg border border-default bg-default/75 p-3 shadow-sm backdrop-blur"
	>
		<div class="grid min-w-0 gap-2">
			<div class="flex items-center justify-between gap-2.5">
				<label class="text-xs text-muted">{{ t('serial.device') }}</label>
				<UBadge
					color="neutral"
					variant="soft"
					class="font-['JetBrains_Mono','MiSans',monospace] text-xs text-muted"
				>
					{{ stateLabel }}
				</UBadge>
			</div>
			<USelectMenu
				v-model="serial.selectedPort"
				:items="portItems"
				value-key="value"
				label-key="label"
				description-key="description"
				:placeholder="t('serial.selectPort')"
			/>
			<div
				class="overflow-hidden text-ellipsis whitespace-nowrap font-['JetBrains_Mono','MiSans',monospace] text-xs text-muted"
			>
				{{ t('serial.hint') }} · {{ t('common.selected') }}: {{ selectedLabel }}
			</div>
		</div>

		<div class="grid content-start gap-2">
			<label class="text-xs text-muted">{{ t('serial.baudRate') }}</label>
			<UInput v-model="baudRate" type="number" :min="9600" />
		</div>

		<div class="grid auto-cols-max grid-flow-col content-start gap-2">
			<UButton color="neutral" variant="soft" type="button" @click="refresh">
				{{ t('common.refresh') }}
			</UButton>
			<UButton color="primary" variant="solid" type="button" @click="connect">
				{{ t('common.connect') }}
			</UButton>
			<UButton color="neutral" variant="soft" type="button" @click="disconnect">
				{{ t('common.disconnect') }}
			</UButton>
		</div>
	</div>
</template>
