<script setup lang="ts">
import SerialConnectionBar from '../controls/SerialConnectionBar.vue'
import { useI18n } from '../../lib/i18n'
import { useSerialStore } from '../../stores/serialStore'

const serial = useSerialStore()
const { t } = useI18n()
</script>

<template>
	<div class="grid h-full min-h-0 grid-rows-[auto_minmax(0,1fr)] gap-3">
		<SerialConnectionBar />

		<div
			class="h-full overflow-hidden rounded-lg border border-default bg-default/75 p-3 font-['JetBrains_Mono','MiSans',monospace] text-[12px] shadow-sm backdrop-blur"
		>
			<div class="mb-2 flex items-center justify-between gap-3 text-muted">
				<span>{{ t(`serial.status.${serial.status.state}`) }}</span>
				<UBadge color="neutral" variant="soft">
					{{ serial.status.selectedPort ?? serial.selectedPort ?? '-' }}
				</UBadge>
			</div>
			<UScrollArea class="h-full">
				<div
					v-for="line in serial.lines"
					:key="line.id"
					class="grid grid-cols-[90px_1fr] gap-3 border-b border-default py-1.5 text-muted"
					:class="{
						'text-warning': line.level === 'warning',
						'text-error': line.level === 'error',
					}"
				>
					<span>{{ new Date(line.timestamp).toLocaleTimeString() }}</span>
					<span>{{ line.message ?? line.line }}</span>
					<code
						v-if="line.message"
						class="col-start-2 whitespace-pre-wrap text-muted"
						>{{ line.line }}</code
					>
				</div>
			</UScrollArea>
		</div>
	</div>
</template>
